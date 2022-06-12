#include "scheduler.h"
#include "simplecsv.h"
#include "inputfiles_projects.h"
#include "workdate.h"
#include "globallogger.h"

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

        ciSet depNdxs = mItems[node].mExpandedDependencyIndices; // take a copy.
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

        for (auto & itm : mItems)
            itm.mExpandedDependencyIndices.clear(); // is garbage since we've changed the order!
        for (auto & proj : mProjects)
            proj.mAllItemIndices.clear(); // also garbage.
    }

    void scheduler::_prioritiseAndMergeTeams()
    {
        ASSERT(mItems.size()==0);

        // copy to mItems in order respecting team and project priority.
        std::vector<unsigned int> positions(mI.mTeamBacklogs.mTeamItems.size(), 0);
        unsigned int numItems = 0;
        for (auto &t : mI.mTeamBacklogs.mTeamItems)
            numItems += t.size();
        ASSERT(numItems > 0);
        ASSERT(mItems.size() == 0);
        for (unsigned int c = 0; c < numItems; ++c)
        { // find a backlog item and move it across.
            const inputfiles::backlogitem *topt = NULL;
            unsigned int bestProjNdx = UINT_MAX;
            unsigned int bestTeam = UINT_MAX;

            for (unsigned int x = 0; x < mI.mTeamBacklogs.mTeamItems.size(); ++x)
            {
                auto &t = mI.mTeamBacklogs.mTeamItems[x];
                if (t.size() > positions[x])
                {
                    auto &tt = t.at(positions[x]);

                    unsigned int projNdx = mI.mProjects.getIndexByID(tt.mProjectName);
                    if (projNdx==eNotFound)
                        TERMINATE(S()<<"No project \""<<(tt.mProjectName)<<"\" ... referred to by \""<<tt.mDescription<<"\"");
                    if ((topt == NULL || projNdx < bestProjNdx))
                    {
                        topt = &tt;
                        bestProjNdx = projNdx;
                        bestTeam = x;
                    }
                }
            }

            scheduleditem si(*topt, c, bestProjNdx, positions[bestTeam]);
            mItems.push_back(si);
            positions[bestTeam] += 1;
        }
        ASSERT(mItems.size() == numItems);

        // then sort by priorty.
        std::sort(mItems.begin(), mItems.end(), comparePriority);

        logdebug(S()<<"Priortised and Merged " << numItems << " items.");
    }

    void scheduler::_determinestart_and_dotask(unsigned int backlogitemNdx)
    {
        auto &z = mItems[backlogitemNdx];
        // set start date based on EarliestStart
        z.mActualStart = z.mEarliestStart;
        ASSERT(!z.mActualStart.isForever());

        // now update based on dependencies
        for (unsigned int k : z.mExpandedDependencyIndices)
        {
            if (k > backlogitemNdx)
                TERMINATE(S() << "Out of order dependency : " << mItems[k].mId << " is needed for "
                              << z.mId << " (" << z.getFullName() << ")");
            if (mItems[k].mActualEnd >= z.mActualStart)
            {
                z.mBlockedBy = (mItems[k].mId.length()>0 ? mItems[k].mId : mItems[k].mDescription);
                z.mActualStart = mItems[k].mActualEnd; // intervals are half open.
            }
        }

        // and update schedule based on resourcing (blocking and contributing).
        if (z.mResources.size() == 0 || z.mDevCentiDays == 0)
        { // no internal resources! Hurrah.
            z.mActualEnd = z.mActualStart + std::max(z.mDevCentiDays.getRoundUpDays(), z.mMinCalendarDays);
            z.mClosedEnd = z.mActualEnd;
            if (z.mClosedEnd>z.mActualStart)
                z.mClosedEnd.decrementWorkDay();
        }
        else
        { // manage resources (people!)
            workdate minstart;
            minstart.setForever();
            unsigned int blockcount=0;
            // 1. adjust start by blocking resources, ensuring *all* blockers are available to start together.
            for (int pi = 0; pi < (int)z.mResources.size(); ++pi)
                {
                    scheduledperson &p = getPersonByName(z.mResources[pi].mName);
                    workdate pstart = p.getEarliestStart(z.mActualStart);

                    if (!pstart.isForever() && pstart<minstart)
                        minstart=pstart;

                    if (z.mResources[pi].mBlocking)
                    {
                        ++blockcount;
                        if (pstart.isForever())
                            TERMINATE(p.mName + " is blocking but can never start task " + z.getFullName());
                        if (pstart > z.mActualStart)
                        {
                            z.mActualStart = pstart;
                            z.mBlockedBy = p.mName;
                            pi = -1; // start over, finding the first date *everyone* blocking has some availability!
                        }
                    }
                }
            if (blockcount==0)
            {
                //logdebug(S()<<"minstart="<<minstart.getStr()<<"  actualstart="<<z.mActualStart.getStr()<<std::endl<<z.getFullName()<<std::endl<<std::endl);
                z.mActualStart = std::max(z.mActualStart,minstart); // handle the case where there is nobody blocking -- start is the first day someone (anyone) is available!
            }

            // 2. do the task.
            _dotask_v2(backlogitemNdx);
        }
    }

    tCentiDay sumVec( const std::vector<tCentiDay> & x )
    {
        tCentiDay rval;
        for (auto i : x)
            rval+=i;
        return rval;
    }
    unsigned long maxNdx(const std::vector<tCentiDay> & x )
    {
        tCentiDay max=0;
        unsigned long maxndx=0;
        for (unsigned long i=0;i<x.size();++i)      
            if (x[i]>max)
            {
                max=x[i];
                maxndx=i;
            }
        return maxndx;
    }

    void scheduler::_dotask_v2_limitedassign(unsigned int itemNdx, tCentiDay &remainTeamToday, std::vector<tCentiDay> &sumCentiDays, std::vector<tCentiDay> &maxCentiDays, tCentiDay &totalDevCentiDaysRemaining, const workdate id)
    {
        scheduleditem &z = mItems[itemNdx];
        ASSERT(remainTeamToday <= totalDevCentiDaysRemaining);
        ASSERT( z.mResources.size()>0 );

        std::vector<tCentiDay> decrements(z.mResources.size(),0);
        std::vector<tCentiDay> availibility(z.mResources.size(),0);

        for (unsigned int pi = 0; pi < z.mResources.size(); ++pi)
        {
            scheduledperson & p = getPersonByName(z.mResources[pi].mName);
            availibility[pi] = p.getAvailability(id);
            decrements[pi] = availibility[pi];
        }

        while (sumVec(decrements) > remainTeamToday)
        { // can't go max speed! Decrease the biggest contribution to even it out.
            decrements[ maxNdx(decrements) ] -= 1;
        }
        
        for (unsigned int pi = 0; pi < z.mResources.size(); ++pi)
        {
            tNdx personNdx = getPersonIndexFromName(z.mResources[pi].mName);
            scheduledperson &p = mPeople[personNdx];
            tCentiDay d = decrements[pi];
            if (d > 0)
            {
                ASSERT(remainTeamToday>=d);
                p.decrementAvailability(id.getDayAsIndex(), d, itemNdx);
                totalDevCentiDaysRemaining -= d;
                remainTeamToday -= d;
                sumCentiDays[pi] += d;
                maxCentiDays[pi] = std::max( maxCentiDays[pi], d);

                mWorkLog.push_back(worklogitem(id.getGregorian(),itemNdx,personNdx,d,totalDevCentiDaysRemaining,p.getAvailability(id)));
            }
        }
    }

    void scheduler::_dotask_v2(unsigned int itemNdx)
    { // More advanced algorithm that moves forward as fast as possible, but not above average for team to finish on time.
        //    Still march day by day, taking up as much availability as we can for each person,
        //    until we have reached the desired devdays.
        scheduleditem &z = mItems[itemNdx];
        std::vector<tCentiDay> sumCentiDays(z.mResources.size(), 0);
        std::vector<tCentiDay> maxCentiDays(z.mResources.size(), 0);
        tCentiDay totalDevCentiDaysRemaining = z.mDevCentiDays;
        workdate id = z.mActualStart;
        z.mClosedEnd = z.mActualStart; // if task requires no work set to same-day.
        ASSERT(!z.mActualStart.isForever());
        while (totalDevCentiDaysRemaining > 0)
        { // loop over days (id)
            tCentiDay remainTeamToday = totalDevCentiDaysRemaining; // each team member capped at 100 centidays/day.

            unsigned long calDaysPast = workdate::countWorkDays(z.mActualStart,id);
            if (z.mMinCalendarDays > calDaysPast)
            {
                unsigned long calDaysRemain = z.mMinCalendarDays - calDaysPast; // includes today.
                tCentiDay maxPaceToday = (tCentiDay)(0.5 + (double)totalDevCentiDaysRemaining / (double)calDaysRemain);
                remainTeamToday = std::min(remainTeamToday, maxPaceToday);
            }

            // assign what we can today.
            _dotask_v2_limitedassign(itemNdx, remainTeamToday, sumCentiDays, maxCentiDays, totalDevCentiDaysRemaining, id);

            z.mClosedEnd=id;
            id.incrementWorkDay();
        }
        z.mActualEnd = id;
        ASSERT(z.mClosedEnd>=z.mActualStart);

        //double duration = z.getDurationDays();
        for (unsigned int pi = 0; pi < z.mLoadingPercent.size(); ++pi)
        {
            z.mLoadingPercent[pi] = maxCentiDays[pi]; //sumCentiDays[pi].getL() / duration;
            z.mTotalContribution[pi]=sumCentiDays[pi];
        }
    }

    void scheduler::_expandDependencies()
    {
        for (auto & itm : mItems)
            ASSERT(itm.mExpandedDependencyIndices.size()==0);
        for (auto & proj : mProjects)
            ASSERT(proj.mAllItemIndices.size()==0);

        for (unsigned int i=0;i<mItems.size();++i)
            {
                auto & item = mItems[i];
                mProjects[item.mProjectIndex].mAllItemIndices.add(i);
            }
        
        for (unsigned int i=0;i<mItems.size();++i)
            for (unsigned int j=0;j<mItems[i].mDependencies.size();++j)
            {
                auto & dep = mItems[i].mDependencies[j];
                unsigned int ndx = getProjectIndexFromId(dep);
                if (ndx!=eNotFound)
                {
                    mItems[i].mExpandedDependencyIndices.add( mProjects[ndx].mAllItemIndices );
                } 
                else
                {
                    ndx = getItemIndexFromId(dep); /// or eNotFound
                    if (ndx==eNotFound)
                        TERMINATE(S()<<" Could not find dependency "<<dep);
                    mItems[i].mExpandedDependencyIndices.add(ndx);
                }
            }
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
        _expandDependencies();      // turn project dependencies into individual item dependencies.
        _topological_sort();        // sort based on task dependency directed graph.
        _expandDependencies();      // order has changed - recreate!

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
        mWorkLog.clear();
    }


    void scheduler::_prepare_to_schedule()
    {
        // copy people name/maxtime from the teams list into our people list.
        for (const auto &x : mI.mTeams)
            for (const auto &y : x.mMembers)
                mPeople.push_back(scheduledperson(y, mI.mHolidays));

        // copy projects
        for (const auto &p : mI.mProjects)
            mProjects.push_back(scheduledproject(p));
    }


    void scheduler::_calc_project_summary()
    { // scheduling done.
        ASSERT(mScheduled);

        for (auto &z : mProjects)
        {
            z.mTotalDevCentiDays = 0;
            z.mActualStart.setForever();
            z.mActualEnd.setToStart();
            z.mClosedEnd.setToStart();

            z.mAggregatedTags.mergefrom( z.getTags() );
        }

        // iterate through tasks, taking max and min duration.
        for (const auto &task : mItems)
        {
            auto &p = mProjects[task.mProjectIndex];

            if (task.mActualStart < p.mActualStart)
                p.mActualStart = task.mActualStart;

            if (task.mActualEnd > p.mActualEnd)
                p.mActualEnd = task.mActualEnd;

            if (task.mClosedEnd > p.mClosedEnd)
                p.mClosedEnd = task.mClosedEnd;

            ASSERT(task.mClosedEnd>=task.mActualStart);

//            if (task.mResources.size() == 0)
                p.mTotalDevCentiDays += task.mDevCentiDays;
            // else
            //     for (auto &x : task.mTotalContribution) // total contribution per resource
            //         p.mTotalDevCentiDays += x;

            p.mContributors.mergefromT<inputfiles::resource>(task.mResources); 
            p.mAggregatedTags.mergefrom(task.mTags);
        }

        for (auto &proj : mProjects)
            if (proj.mActualStart.isForever()) // no tasks.
                proj.mActualStart.setToStart();
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

        loginfo(S() << "Scheduling "<<mI.mTeamBacklogs.getTotalNumItems()<<" backlog items across "<< mI.mTeams.size()<<" teams.");
        timer t;

        _prepare_to_schedule();
        _schedule();
        _calc_project_summary();

        logdebug(S()<<"Time taken: " << std::setprecision(3) << t.stop() << " ms.");
        logdebug("Scheduling complete.");
    }


} // namespace