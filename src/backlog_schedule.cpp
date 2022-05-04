
#include "backlog.h"

// Compares two items by priority
bool comparePriority(const backlogitem & i1, const backlogitem & i2)
{
    return (i1.mPriority < i2.mPriority);
}


void backlog::_topological_visit(int node, std::vector<bool> & tempMarks, std::vector<bool> & permMarks, std::vector<unsigned int> & scheduledList)
{
    if (permMarks[node]) return;
    if (tempMarks[node]) TERMINATE("Not a directed acyclic graph - circular dependency involving "+mItems[node].mId);

    tempMarks[node]=true;

    std::vector<int> depNdxs;
    for (auto & d : mItems[node].mDependencies)
        {
            unsigned int k = getItemIndexFromId(d);
            if (k==eNotFound) TERMINATE( S() << "The dependency \""<< d <<"\" does not exist!");
            depNdxs.push_back(k); 
        }    
    std::sort(depNdxs.begin(),depNdxs.end());
    for (auto d : depNdxs)
        _topological_visit(d,tempMarks,permMarks,scheduledList);
    
    tempMarks[node]=false;
    permMarks[node]=true;
    scheduledList.push_back(node);
}

// depth first search.
void backlog::_topological_sort()
{
    std::vector<bool> tempMarks(mItems.size(),false);
    std::vector<bool> permMarks(mItems.size(),false);
    std::vector<unsigned int> scheduledList;

    for (unsigned int i=0;i<mItems.size();++i)
        if (!permMarks[i]) 
            _topological_visit(i,tempMarks,permMarks,scheduledList);

    ASSERT(scheduledList.size()==mItems.size());
    for (unsigned int i=0; i<scheduledList.size();++i)
        mItems[scheduledList[i]].mPriority=i;

    std::sort( mItems.begin(), mItems.end(), comparePriority);
}



void backlog::_prioritiseAndMergeTeams()
{
    unsigned int numItems=0;
    for (auto & t:mTeamsItems)
        numItems += t.size();
    
    ASSERT(numItems>0);
    ASSERT(mItems.size()==0);
    for (unsigned int c=0;c<numItems;++c)
    { // find a backlog item and move it across.
        std::deque<backlogitem> * topt = NULL;
        for (auto & t : mTeamsItems)
            if (t.size()>0 && (topt==NULL || t[0].mProject < topt->at(0).mProject))
                topt = & t;
        topt->at(0).mPriority = c;
        topt->at(0).mMergePriority = c;
        mItems.push_back( topt->at(0) );
        topt->pop_front();
    }
    ASSERT(mItems.size()==numItems);

    // first sort by priorty.
    std::sort( mItems.begin(), mItems.end(), comparePriority);

    std::cout << "Priortised and Merged "<<numItems<<" items."<<std::endl;
}


void backlog::_schedule()
{
    _prioritiseAndMergeTeams();
    _topological_sort(); // sort based on dependency directed graph.

    // now figure out star and end dates for the items, reserving resources as we go.
    for (unsigned int zndx=0; zndx<mItems.size();++zndx)
    {
        auto & z = mItems[zndx];
        // set start date based on EarliestStart
        z.mActualStart = z.mEarliestStart;
        ASSERT(!z.mActualStart.isForever());

        // now update based on dependencies
        for (unsigned int j=0;j<z.mDependencies.size();j++)
        {
            unsigned int k = getItemIndexFromId(z.mDependencies[j]);
            if (k==eNotFound) TERMINATE("programming error - couldn't find already found dependency "+z.mDependencies[j]);
            if (k>zndx)
                TERMINATE(S() << "Out of order dependency : "<<z.mDependencies[j]<<" is needed for "
                    << z.mId << " (" << z.getFullName() << ")");
            if (mItems[k].mActualEnd >= z.mActualStart)
                z.mActualStart=mItems[k].mActualEnd; // intervals are half open.
        }

        // and update schedule based on resourcing (blocking and contributing).
        if (z.mResources.size()==0 || z.mDevDays==0) // no internal resources! Hurrah.
        {
            z.mActualEnd=z.mActualStart+std::max(z.mDevDays,z.mMinCalendarDays);
        }
        else
        {  // manage resources (people!)
            // 1. adjust start by blocking resources, ensuring *all* blockers are available to start together.
            for (int pi=0;pi<(int)z.mResources.size();++pi)
                if (z.mResources[pi].mBlocking)
                    {
                        person & p = getPersonByName(z.mResources[pi].mName);
                        itemdate pstart = p.getEarliestStart(z.mActualStart);
                        
                        if (pstart.isForever())
                            TERMINATE(p.mName+" is blocking but can never start task "+z.getFullName());                         

                        if (pstart>z.mActualStart)
                        {
                            z.mActualStart = pstart;
                            z.mBlockedBy = p.mName;
                            pi=-1; // start over, finding the first date *everyone* blocking has some availability!
                        }
                    }

            // 2. Assign individual resourcing, and determine end date for task.

            //    Limit rate (dd/d) by  devdays/(n * calendar days), so we can't outpace the task.
            tCentiDay maxindividualrate = 100;
            if (z.mMinCalendarDays>0) 
                maxindividualrate = (int)(0.5 + 100.0 * (double)z.mDevDays / (double)( z.mResources.size() * z.mMinCalendarDays ));

            //    Now march day by day, taking up as much availability as we can for each person,
            //    until we have reached the desired devdays. 
            std::vector<double> sumCentiDays(z.mResources.size(),0.0);
            tCentiDay totalDevCentiDaysRemaining = 100 * z.mDevDays;
            itemdate id=z.mActualStart;
            ASSERT(!z.mActualStart.isForever());
            while (totalDevCentiDaysRemaining>0)
            { // loop over days (id)
                for (unsigned int pi=0;pi<z.mResources.size();++pi)
                {
                    person & p = getPersonByName(z.mResources[pi].mName);
                    tCentiDay d = p.getAvailability(id);
                    if (d>totalDevCentiDaysRemaining) d=totalDevCentiDaysRemaining;
                    if (d>maxindividualrate) d=maxindividualrate;
                    p.decrementAvailability(id,d, z.getFullName());
                    totalDevCentiDaysRemaining-=d;
                    sumCentiDays[pi]+=d;
                }
                ++id;
            }
            z.mActualEnd = id;

            double duration = (z.mActualEnd-z.mActualStart).getAsDurationDouble();
            for (unsigned int pi=0;pi<z.mResources.size();++pi)
                z.mResources[pi].mLoadingPercent = sumCentiDays[pi]/duration;

        ASSERT(!z.mActualStart.isForever());
        }
    }

    // printorder(mItems);
}