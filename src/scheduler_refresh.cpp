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

    void scheduler::advance(itemdate newStart, inputfiles::inputset &iset)
    {
        itemdate oldStart;
        oldStart.setToStart();

        // The main event: update the backlog items in the team files.
        ASSERT(!mScheduled); // assume we've not scheduled.

        schedule();

        iset.mH.advance(newStart); // just drops old holidays
        iset.mP.advance(newStart); // does nothing at present
        iset.mT.advance(newStart); // advances leave

        gSettings().advance(newStart);

        // // advance mItems.
        // std::vector<tCentiDay> itemDevDone(mItems.size(),0);
        // for (auto & p : mPeople)
        // {
        //     for (itemdate d = today; d < newStart ; ++d)
        //     {
        //         const std::vector<daychunk> & chunks( p.getChunks(d.getDayAsIndex()) );
        //         for (auto & c : chunks)
        //         {
        //             itemDevDone[c.mItemIndex] += c.mEffort;
        //         }
        //     }
        // }

        // for (unsigned int i =0 ; i< mItems.size(); ++i)
        // {
        //     mItems[i].mDevDays -= (int)(0.5 + 0.01*itemDevDone[i]);

        //     if (mItems[i].mActualStart < newStart)
        //         {
        //             if (newStart - mItems[i].mActualStart >= mItems[i].mMinCalendarDays)
        //                 mItems[i].mMinCalendarDays=0;
        //             else
        //                 mItems[i].mMinCalendarDays -= (newStart - mItems[i].mActualStart).getAsDurationUInt();
        //         }

        //     if (mItems[i].mActualStart < newStart)
        //         mItems[i].mActualStart = newStart;
        //     if (mItems[i].mActualEnd < newStart)
        //         mItems[i].mActualEnd = newStart;
        // }
    }

} // namespace