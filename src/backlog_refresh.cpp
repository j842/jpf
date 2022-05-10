#include "backlog.h"

void backlog::refresh()
{
    { // remove any unused refrences.
        std::vector<int> refcount(mItems.size(), 0);
        for (auto &i : mItems)
            for (auto &j : i.mDependencies)
            {
                unsigned int n = getItemIndexFromId(j);
                refcount[n] += 1;
            }

        for (unsigned int ii = 0; ii < mItems.size(); ii++)
            if (refcount[ii] == 0)
                mItems[ii].mId = "";
    }
}
