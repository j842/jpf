
#include "scheduler.h"
#include "simplecsv.h"
#include "inputfiles_projects.h"

namespace scheduler

{

    // Compares two items by priority
    bool comparePriority(const scheduleditem &i1, const scheduleditem &i2)
    {
        return (i1.mPriority < i2.mPriority);
    }

    void scheduler::_topological_visit(int node, std::vector<bool> &tempMarks, std::vector<bool> &permMarks, std::vector<unsigned int> &scheduledList)
    {
        if (permMarks[node])
            return;
        if (tempMarks[node])
            TERMINATE("Not a directed acyclic graph - circular dependency involving " + mItems[node].mId);

        tempMarks[node] = true;

        std::vector<int> depNdxs;
        for (auto &d : mItems[node].mDependencies)
        {
            unsigned int k = getItemIndexFromId(d);
            if (k == eNotFound)
                TERMINATE(S() << "The dependency \"" << d << "\" does not exist!");
            depNdxs.push_back(k);
        }
        std::sort(depNdxs.begin(), depNdxs.end());
        for (auto d : depNdxs)
            _topological_visit(d, tempMarks, permMarks, scheduledList);

        tempMarks[node] = false;
        permMarks[node] = true;
        scheduledList.push_back(node);
    }

    // depth first search.
    void scheduler::_topological_sort()
    {
        std::vector<bool> tempMarks(mItems.size(), false);
        std::vector<bool> permMarks(mItems.size(), false);
        std::vector<unsigned int> scheduledList;

        for (unsigned int i = 0; i < mItems.size(); ++i)
            if (!permMarks[i])
                _topological_visit(i, tempMarks, permMarks, scheduledList);

        ASSERT(scheduledList.size() == mItems.size());
        for (unsigned int i = 0; i < scheduledList.size(); ++i)
            mItems[scheduledList[i]].mPriority = i;

        std::sort(mItems.begin(), mItems.end(), comparePriority);
    }

    void scheduler::_prioritiseAndMergeTeams()
    {
        mItems.clear();

        // copy to mItems in order respecting team and project priority.
        std::vector<unsigned int> positions(mI.mB.mTeamItems.size(), 0);
        unsigned int numItems = 0;
        for (auto &t : mI.mB.mTeamItems)
            numItems += t.size();
        ASSERT(numItems > 0);
        ASSERT(mItems.size() == 0);
        for (unsigned int c = 0; c < numItems; ++c)
        { // find a backlog item and move it across.
            const inputfiles::backlogitem *topt = NULL;
            unsigned int bestProjNdx = UINT_MAX;
            unsigned int bestTeam = UINT_MAX;

            for (unsigned int x = 0; x < mI.mB.mTeamItems.size(); ++x)
            {
                auto &t = mI.mB.mTeamItems[x];
                if (t.size() > positions[x])
                {
                    auto &tt = t.at(positions[x]);

                    unsigned int projNdx = projects().getIndexByID(tt.mProjectName);
                    if ((topt == NULL || projNdx < bestProjNdx))
                    {
                        topt = &tt;
                        bestProjNdx = projNdx;
                        bestTeam = x;
                    }
                }
            }

            scheduleditem si(*topt, c, bestProjNdx);
            mItems.push_back(si);
            positions[bestTeam] += 1;
        }
        ASSERT(mItems.size() == numItems);

        // then sort by priorty.
        std::sort(mItems.begin(), mItems.end(), comparePriority);

        std::cout << "Priortised and Merged " << numItems << " items." << std::endl;
    }

    void scheduler::_determinestart_and_dotask(unsigned int backlogitemNdx)
    {
        auto &z = mItems[backlogitemNdx];
        // set start date based on EarliestStart
        z.mActualStart = z.mEarliestStart;
        ASSERT(!z.mActualStart.isForever());

        // now update based on dependencies
        for (unsigned int j = 0; j < z.mDependencies.size(); j++)
        {
            unsigned int k = getItemIndexFromId(z.mDependencies[j]);
            if (k == eNotFound)
                TERMINATE("programming error - couldn't find already found dependency " + z.mDependencies[j]);
            if (k > backlogitemNdx)
                TERMINATE(S() << "Out of order dependency : " << z.mDependencies[j] << " is needed for "
                              << z.mId << " (" << z.getFullName() << ")");
            if (mItems[k].mActualEnd >= z.mActualStart)
                z.mActualStart = mItems[k].mActualEnd; // intervals are half open.
        }

        // and update schedule based on resourcing (blocking and contributing).
        if (z.mResources.size() == 0 || z.mDevDays == 0)
        { // no internal resources! Hurrah.
            z.mActualEnd = z.mActualStart + std::max(z.mDevDays, z.mMinCalendarDays);
        }
        else
        { // manage resources (people!)
            // 1. adjust start by blocking resources, ensuring *all* blockers are available to start together.
            for (int pi = 0; pi < (int)z.mResources.size(); ++pi)
                if (z.mResources[pi].mBlocking)
                {
                    person &p = getPersonByName(z.mResources[pi].mName);
                    itemdate pstart = p.getEarliestStart(z.mActualStart);

                    if (pstart.isForever())
                        TERMINATE(p.mName + " is blocking but can never start task " + z.getFullName());

                    if (pstart > z.mActualStart)
                    {
                        z.mActualStart = pstart;
                        z.mBlockedBy = p.mName;
                        pi = -1; // start over, finding the first date *everyone* blocking has some availability!
                    }
                }

            // 2. do the task.
            _dotask_v2(backlogitemNdx);
        }
    }

    void scheduler::_dotask_v2_limitedassign(unsigned int itemNdx, const tCentiDay maxAllocation, tCentiDay &remainTeamToday, std::vector<double> &sumCentiDays, tCentiDay &totalDevCentiDaysRemaining, const itemdate id)
    {
        scheduleditem &z = mItems[itemNdx];
        ASSERT(maxAllocation <= remainTeamToday);
        for (unsigned int pi = 0; pi < z.mResources.size(); ++pi)
        {
            person &p = getPersonByName(z.mResources[pi].mName);
            tCentiDay d = std::min(p.getAvailability(id), maxAllocation);
            if (d > 0)
            {
                p.decrementAvailability(id, d, itemNdx);
                totalDevCentiDaysRemaining -= d;
                remainTeamToday -= d;
                sumCentiDays[pi] += d;
            }
        }
    }

    void scheduler::_dotask_v2(unsigned int itemNdx)
    { // More advanced algorithm that moves forward as fast as possible, but not above average for team to finish on time.
        //    Still march day by day, taking up as much availability as we can for each person,
        //    until we have reached the desired devdays.
        scheduleditem &z = mItems[itemNdx];
        std::vector<double> sumCentiDays(z.mResources.size(), 0.0);
        tCentiDay totalDevCentiDaysRemaining = 100 * z.mDevDays;
        itemdate id = z.mActualStart;
        ASSERT(!z.mActualStart.isForever());
        while (totalDevCentiDaysRemaining > 0)
        { // loop over days (id)
            unsigned int calDaysPast = (id - z.mActualStart).getAsDurationUInt();
            unsigned int calDaysRemain = 0;
            if (z.mMinCalendarDays > calDaysPast)
                calDaysRemain = z.mMinCalendarDays - calDaysPast; // includes today.

            tCentiDay maxTeamToday = std::min(totalDevCentiDaysRemaining, (tCentiDay)(100 * z.mResources.size()));
            if (calDaysRemain > 0)
            { // check if we're going too fast to finish in the calendar days remaining.
                tCentiDay maxSpeedRemain = (tCentiDay)((double)totalDevCentiDaysRemaining / (double)calDaysRemain);
                maxTeamToday = std::min(maxTeamToday, maxSpeedRemain);
            }
            tCentiDay remainTeamToday = maxTeamToday;

            // no we need to see if we can allocate maxTeamToday centidays amongst the resources...
            tCentiDay evenPace = maxTeamToday / (z.mResources.size()); // if everyone could go as fast as we like, this is how fast we'd go.

            // go through once limiting to an even pace.
            _dotask_v2_limitedassign(itemNdx, evenPace, remainTeamToday, sumCentiDays, totalDevCentiDaysRemaining, id);
            // now go through again and overallocate (above evenPace) anything left over if we can. Will bias towards first resouce but good enough.
            _dotask_v2_limitedassign(itemNdx, remainTeamToday, remainTeamToday, sumCentiDays, totalDevCentiDaysRemaining, id);

            ++id;
        }
        z.mActualEnd = id;

        double duration = (z.mActualEnd - z.mActualStart).getAsDurationDouble();
        for (unsigned int pi = 0; pi < z.mResources.size(); ++pi)
            z.mResources[pi].mLoadingPercent = sumCentiDays[pi] / duration;
    }

    // ---------------------------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------------------------------------------

    void scheduler::_schedule()
    {
        if (mScheduled)
            TERMINATE("Attempting to schedule multiple times without resetting schedule - programming error.");

        // these two order the tasks optimally.
        _prioritiseAndMergeTeams(); // merge the tasks from each team list together, based on project priorities.
        _topological_sort();        // sort based on task dependency directed graph.

        // now figure out start and end dates for the items, based on human resourcing, reserving resources as we go.
        for (unsigned int zndx = 0; zndx < mItems.size(); ++zndx)
        {
            _determinestart_and_dotask(zndx);
            ASSERT(!mItems[zndx].mActualStart.isForever());
        }

        mScheduled = true;
    }

    void scheduler::resetSchedule()
    { // reset.
        mScheduled = false;
        mPeople.clear();
        mItems.clear();
        mProjects.clear();
    }


    void scheduler::_prepare_to_schedule()
    {
        // copy people name/maxtime from the teams list into our people list.
        for (const auto &x : mI.mT)
            for (const auto &y : x.mMembers)
                mPeople.push_back(person(y, mI.mH));

        // copy projects
        for (const auto &p : mI.mP)
            mProjects.push_back(scheduledproject(p));
    }


    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------

    void scheduler::schedule()
    {
        if (mScheduled)
            TERMINATE("Attempting to schedule multiple times without resetting schedule - programming error.");
        ASSERT(mPeople.size() == 0);
        ASSERT(mItems.size() == 0);
        ASSERT(mProjects.size() == 0);

        std::cout << "Scheduling backlog items..." << std::endl;
        timer t;

        _prepare_to_schedule();
        _schedule();
        _calc_project_summary();

        std::cout << "Scheduling done in " << std::setprecision(3) << t.stop() << " ms." << std::endl;
    }


} // namespace