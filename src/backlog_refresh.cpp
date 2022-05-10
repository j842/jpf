#include "backlog.h"

void backlog::refresh()
{
    { // remove any unused refrences.
        unsigned int removedRefs=0;
        std::vector<int> refcount(mItems.size(), 0);
        for (auto &i : mItems)
            for (auto &j : i.mDependencies)
            {
                unsigned int n = getItemIndexFromId(j);
                refcount[n] += 1;
            }

        for (unsigned int ii = 0; ii < mItems.size(); ii++)
            if (refcount[ii] == 0)
                if (mItems[ii].mId.length()>0)
                {
                    mItems[ii].mId = "";
                    ++removedRefs;
                }

        if (removedRefs>0)
            std::cout << "Removed "<<removedRefs<< " unneeded references." << std::endl;
        else 
            std::cout << "References are clean." << std::endl;
    }
}
