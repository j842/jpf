#include "backlog.h"

void backlog::refresh()
{
    _prioritiseAndMergeTeams();     // merge the tasks from each team list together, based on project priorities.

    { // remove any unused refrences.
        std::vector<unsigned int> teamitemcount(mTeams.size(), 0);
        std::vector<std::string> newRefs(mItems.size(),"");

        unsigned int removedRefs = 0;
        std::vector<int> refcount(mItems.size(), 0);
        for (auto &i : mItems)
            for (auto &j : i.mDependencies)
            {
                unsigned int n = getItemIndexFromId(j);
                refcount[n] += 1;
            }

        for (unsigned int ii = 0; ii < mItems.size(); ii++)
        {
            if (refcount[ii] == 0)
            {
                if (mItems[ii].mId.length() > 0)
                {
                    mItems[ii].mId = "";
                    ++removedRefs;
                }
            }
            else
            {
                std::string old = mItems[ii].mId;
                unsigned int teamndx = mItems[ii].mTeamNdx;
                teamitemcount[teamndx] += 1;
                newRefs[ii] = (S() << mTeams[teamndx].mRefCode << std::setw(2) << std::setfill('0') << teamitemcount[teamndx]);
            }
        }

        for (auto & i : mItems)
            for (auto & j : i.mDependencies)
                j = newRefs[getItemIndexFromId(j)];

        for (unsigned int x=0 ; x < mItems.size() ; ++x)
            mItems[x].mId = newRefs[x];

        if (removedRefs > 0)
            std::cout << "Removed " << removedRefs << " unneeded reference" << (removedRefs > 1 ? "s." : ".") << std::endl;
        else
            std::cout << "References are clean." << std::endl;
    }
}
