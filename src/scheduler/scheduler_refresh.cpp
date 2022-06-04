#include "scheduler.h"
#include "inputfiles_inputset.h"
#include "settings.h"
#include "utils.h"

namespace scheduler
{
    class lowercasecountedmap
    {
        public:
            lowercasecountedmap() {}

            std::string get(std::string key) {return mV[makelower(key)];}
            int getcount(std::string key) {return mC[makelower(key)];}
            
            void set(std::string key, std::string value) {mV[makelower(key)]=value; }
            void addcount(std::string key) {mC[makelower(key)]+=1;} // insert of 0 on non-exist.

        private:
            std::map<std::string,std::string> mV;
            std::map<std::string,int> mC;
    };

    // this version labels only items referenced by other items.
    // void scheduler::refresh(inputfiles::inputset &iset)
    // {
    //     if (mScheduled)
    //         TERMINATE("Error: refresh called on dirty scheduler. Items will be out of order.");

    //     _prioritiseAndMergeTeams(); // merge the tasks from each team list together, based on project priorities and preserving team ordering.

    //     lowercasecountedmap refMap;

    //     { // 1. remove any unused refrences.
    //         std::vector<unsigned int> teamitemcount(iset.mT.size(), 0);

    //         unsigned int removedRefs = 0;
    //         for (auto &i : mItems)
    //             for (auto &j : i.mDependencies)
    //                 refMap.addcount(j);

    //         for (auto &i : mItems)
    //             if (i.mId.length() > 0 && refMap.getcount(i.mId)==0)
    //             {
    //                 i.mId.clear();
    //                 ++removedRefs;
    //             }
    //         if (removedRefs > 0)
    //             loginfo(S()<< "Removed " << removedRefs << " unneeded reference" << (removedRefs > 1 ? "s." : "."));
    //         else
    //             loginfo("References are clean.");
    //     }

    //     { // 2. Renumber remaining Refs.
    //         std::vector<int> teamItemCount(iset.mT.size(), 0);
    //         for (auto &i : mItems)
    //             if (i.mId.length() > 0)
    //             {
    //                 ASSERT(refMap.getcount(i.mId)>0);
    //                 teamItemCount[i.mTeamNdx] += 1;
    //                 refMap.set(
    //                     i.mId, 
    //                     S() << iset.mT[i.mTeamNdx].mRefCode << "." << std::setw(2) << std::setfill('0') << teamItemCount[i.mTeamNdx]
    //                     );
    //                 i.mId = refMap.get(i.mId);
    //             }
    //         for (auto &i : mItems)
    //             for (auto &j : i.mDependencies)
    //                 j = refMap.get(j);
    //     }

    //     { // 3. Fix the team backlogs now with the new references.
    //         for (auto &t0 : iset.mB.mTeamItems)
    //             for (auto &bl : t0)
    //             {
    //                 if (bl.mId.length() > 0)
    //                     bl.mId = refMap.get(bl.mId);

    //                 for (auto &dep : bl.mDependencies)
    //                     dep = refMap.get(dep);
    //             }
    //     }
    // }

// this version renumbers everything.
void scheduler::refresh(inputfiles::inputset &iset)
    {
        if (mScheduled)
            TERMINATE("Error: refresh called on dirty scheduler. Items will be out of order.");

        _prioritiseAndMergeTeams(); // merge the tasks from each team list together, based on project priorities and preserving team ordering.

        std::vector<std::string> refMap(mItems.size(),"");

        { // 1. Determine new numbers.
            std::vector<int> teamItemCount(iset.mT.size(), 0);
            for (unsigned int i=0; i<mItems.size();++i)
            {
                teamItemCount[mItems[i].mTeamNdx] += 1;
                refMap[i] = S() << iset.mT[mItems[i].mTeamNdx].mRefCode << "." << std::setw(2) << std::setfill('0') << teamItemCount[mItems[i].mTeamNdx];

                if (!iSame(refMap[i],mItems[i].mId))
                    logdebug(S()<<"Changing Task ID ["<<mItems[i].mId<<"] to ["<<refMap[i]<<"].");
            }

        }

        { //2 . renumber dependency lists
            for (auto &itm : mItems)
                for (auto &j : itm.mDependencies)
                {
                    unsigned int ndx = getItemIndexFromId(j);
                    if (ndx!=eNotFound)
                    { // otherwise it's a project - leave it alone.
                        ASSERT(ndx>=0 && ndx<mItems.size());
                        j = refMap[ndx];
                    }
                }
        }

        { // 3. Fix the team backlogs (in iset) now with the new references.
            for (unsigned int i=0; i<mItems.size();++i)
                {
                    auto & itm = mItems[i];
                    auto & bli = iset.mB.mTeamItems[ itm.mTeamNdx ][  itm.mItemIndexInTeamBacklog ];
                    bli.mId = refMap[i];

                    for (auto &dep : bli.mDependencies)
                    {
                        unsigned int ndx = getItemIndexFromId(dep);
                        if (ndx != eNotFound)
                            dep = refMap[ndx]; // otherwise a project.
                    }

                    // also ensure we're using the project Id not full name.
                    unsigned int ndx = iset.mP.getIndexByID(bli.mProjectName);
                    if (ndx==eNotFound)
                    {
                        logerror(S()<<"Unable to find project ["+bli.mProjectName<<"]!");
                        ASSERT(ndx!=eNotFound);
                    }
                    const std::string & realid = iset.mP[ ndx ].getId();
                    if (!iSame(realid, bli.mProjectName))
                    {
                        logdebug(S()<<"Using project id ["<<realid<<"] instead of full project name for task "<<bli.mId );
                        bli.mProjectName =realid;
                    }
                }
        }

        { // 4. change the item ids now that we're done string matching within them.
        for (unsigned int i=0; i<mItems.size();++i)
            mItems[i].mId = refMap[i];
        }
    }


    void scheduler::advance(workdate newStart, inputfiles::inputset &iset) const
    {
        workdate oldStart;
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
                        itemDevCentiDone[c.mItemIndex] += c.mEffort;
                }
            }
            for (unsigned int itemndx = 0; itemndx < mItems.size(); ++itemndx)
            { // find item and update.
                unsigned int teamndx = mItems[itemndx].mTeamNdx;
                unsigned int teamitemndx = mItems[itemndx].mItemIndexInTeamBacklog;
                auto & bli = iset.mB.mTeamItems[teamndx][teamitemndx];
                bli.mDevCentiDays -= itemDevCentiDone[itemndx];

                if (mItems[itemndx].mActualStart < newStart)
                {
                    unsigned long delta = workdate::countWorkDays(mItems[itemndx].mActualStart, newStart);
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