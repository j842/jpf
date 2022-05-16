#include "scheduler.h"
#include "inputfiles_inputset.h"
#include "settings.h"

namespace scheduler
{

    void scheduler::refresh(inputfiles::inputset &iset)
    {
        if (mScheduled)
            TERMINATE("Error: refresh called on dirty scheduler. Items will be out of order.");

        _prioritiseAndMergeTeams(); // merge the tasks from each team list together, based on project priorities and preserving team ordering.

        std::map<std::string, std::string> refMap;

        { // 1. remove any unused refrences.
            std::vector<unsigned int> teamitemcount(iset.mT.size(), 0);
            std::map<std::string, int> refCounts;

            unsigned int removedRefs = 0;
            for (auto &i : mItems)
                for (auto &j : i.mDependencies)
                    refCounts[j] += 1;

            for (auto &i : mItems)
                if (i.mId.length() > 0 && refCounts[i.mId] == 0)
                {
                    refMap[i.mId] = "";
                    i.mId.clear();
                    ++removedRefs;
                }
            if (removedRefs > 0)
                std::cout << "Removed " << removedRefs << " unneeded reference" << (removedRefs > 1 ? "s." : ".") << std::endl;
            else
                std::cout << "References are clean." << std::endl;
        }

        { // 2. Renumber remaining Refs.
            std::vector<int> teamItemCount(iset.mT.size(), 0);
            for (auto &i : mItems)
                if (i.mId.length() > 0)
                {
                    teamItemCount[i.mTeamNdx] += 1;
                    refMap[i.mId] = (S() << iset.mT[i.mTeamNdx].mRefCode << "." << std::setw(2) << std::setfill('0') << teamItemCount[i.mTeamNdx]);
                    i.mId = refMap[i.mId];
                }
            for (auto &i : mItems)
                for (auto &j : i.mDependencies)
                    j = refMap[j];
        }

        { // 3. Fix the team backlogs now with the new references.
            for (auto &t0 : iset.mB.mTeamItems)
                for (auto &bl : t0)
                {
                    if (bl.mId.length() > 0)
                        bl.mId = refMap[bl.mId];

                    for (auto &dep : bl.mDependencies)
                        dep = refMap[dep];
                }
        }
    }

    void scheduler::advance(itemdate newStart, inputfiles::inputset &iset) const
    {
        itemdate oldStart;
        oldStart.setToStart();

        // The main event: update the backlog items in the team files.
        ASSERT(mScheduled); // assume we've not scheduled.

        // advance mTeamItems devdays and calendardays.
        {
            std::vector<tCentiDay> itemDevCentiDone(mItems.size(), 0);
            for (auto &p : mPeople)
            {
                ASSERT(oldStart.getDayAsIndex()==0);
                for (unsigned long d = 0; d < newStart.getDayAsIndex(); ++d)
                {
                    const std::vector<daychunk> &chunks = p.getChunks(d);
                    for (auto &c : chunks)
                    {
                        itemDevCentiDone[c.mItemIndex] += c.mEffort;
                        //std::cout << mItems[c.mItemIndex].mDescription << " + " << p.mName <<" : " << c.mEffort << "   ->  " << itemDevCentiDone[c.mItemIndex] << " / " << mItems[c.mItemIndex].mDevCentiDays << std::endl;
                    }
                }
            }
            for (unsigned int itemndx = 0; itemndx < mItems.size(); ++itemndx)
            { // find item and update.
                unsigned int teamndx = mItems[itemndx].mTeamNdx;
                unsigned int teamitemndx = mItems[itemndx].mItemIndexInTeamBacklog;
                auto & bli = iset.mB.mTeamItems[teamndx][teamitemndx];
                bli.mDevCentiDays -= itemDevCentiDone[itemndx];
                //std::cout<<bli.getFullName() <<" -> "<<bli.mDevCentiDays << " remaining." << std::endl;

                if (mItems[itemndx].mActualStart < newStart)
                {
                    unsigned long delta = wdduration(mItems[itemndx].mActualStart, newStart);
                    if (delta >= mItems[itemndx].mMinCalendarDays)
                        bli.mMinCalendarDays=0;
                    else
                        bli.mMinCalendarDays -= delta;

                    for (auto & p : bli.mResources)
                        p.mBlocking = false; // item has started, so nobody is blocking now.
                }

                if (bli.mEarliestStart < newStart)
                    bli.mEarliestStart = newStart;
            }
        }

        iset.mH.advance(newStart); // just drops old holidays
        iset.mP.advance(newStart); // does nothing at present
        iset.mT.advance(newStart); // advances leave
    }

} // namespace