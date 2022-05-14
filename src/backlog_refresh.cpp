#include "backlog.h"

void backlog::refresh()
{
    _prioritiseAndMergeTeams();     // merge the tasks from each team list together, based on project priorities.

    { // 1. remove any unused refrences.
        std::vector<unsigned int> teamitemcount(mTeams.size(), 0);
        std::map<std::string,int> refCounts;

        unsigned int removedRefs = 0;
        for (auto &i : mItems)
            for (auto &j : i.mDependencies)
                refCounts[j] += 1;

        for (auto &i : mItems)
            if (i.mId.length()>0 && refCounts[i.mId]==0)
                {
                    i.mId.clear();
                    ++removedRefs;
                }
        if (removedRefs > 0)
            std::cout << "Removed " << removedRefs << " unneeded reference" << (removedRefs > 1 ? "s." : ".") << std::endl;
        else
            std::cout << "References are clean." << std::endl;
    }

    { // 2. Renumber remaining Refs.
        std::map<std::string,std::string> refMap;
        std::vector<int> teamItemCount(mTeams.size(),0);
        for (auto & i : mItems)
            if (i.mId.length()>0)
            {
                teamItemCount[i.mTeamNdx] += 1;
                refMap[i.mId] = (S() << mTeams[i.mTeamNdx].mRefCode << std::setw(2) << std::setfill('0') << teamItemCount[i.mTeamNdx]);
                i.mId = refMap[i.mId];
            }
        for (auto & i : mItems)
            for (auto & j : i.mDependencies)
                j = refMap[j];
    }
}


void backlog::advance(itemdate newStart)
{
    mPubHols.advance(newStart);
    for (auto & z : mItems)
//        z.advance(newStart);
    ;
}
