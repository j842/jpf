
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
        mItems.push_back( topt->at(0) );
        topt->pop_front();
    }
    ASSERT(mItems.size()==numItems);

    // first sort by priorty.
    std::sort( mItems.begin(), mItems.end(), comparePriority);

    std::cout << "Priortised and Merged "<<numItems<<" items."<<std::endl;
}


void backlog::_determinestart_and_dotask(unsigned int backlogitemNdx)
{
    auto & z = mItems[backlogitemNdx];
    // set start date based on EarliestStart
    z.mActualStart = z.mEarliestStart;
    ASSERT(!z.mActualStart.isForever());

    // now update based on dependencies
    for (unsigned int j=0;j<z.mDependencies.size();j++)
    {
        unsigned int k = getItemIndexFromId(z.mDependencies[j]);
        if (k==eNotFound) TERMINATE("programming error - couldn't find already found dependency "+z.mDependencies[j]);
        if (k>backlogitemNdx)
            TERMINATE(S() << "Out of order dependency : "<<z.mDependencies[j]<<" is needed for "
                << z.mId << " (" << z.getFullName() << ")");
        if (mItems[k].mActualEnd >= z.mActualStart)
            z.mActualStart=mItems[k].mActualEnd; // intervals are half open.
    }

    // and update schedule based on resourcing (blocking and contributing).
    if (z.mResources.size()==0 || z.mDevDays==0) 
    { // no internal resources! Hurrah.
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
        
        // 2. do the task.
        _dotask_v2(backlogitemNdx);
    }
}


// void backlog::_dotask_v1(backlogitem & z)
// { // limit all people by the calendar day rate - simple algorithm.
//     //    Limit rate (dd/d) by  devdays/(n * calendar days), so we can't outpace the task.
//     tCentiDay maxindividualrate = 100;
//     if (z.mMinCalendarDays>0) 
//         maxindividualrate = (int)(0.5 + 100.0 * (double)z.mDevDays / (double)( z.mResources.size() * z.mMinCalendarDays ));

//     //    Now march day by day, taking up as much availability as we can for each person,
//     //    until we have reached the desired devdays. 
//     std::vector<double> sumCentiDays(z.mResources.size(),0.0);
//     tCentiDay totalDevCentiDaysRemaining = 100 * z.mDevDays;
//     itemdate id=z.mActualStart;
//     ASSERT(!z.mActualStart.isForever());
//     while (totalDevCentiDaysRemaining>0)
//     { // loop over days (id)
//         for (unsigned int pi=0;pi<z.mResources.size();++pi)
//         {
//             person & p = getPersonByName(z.mResources[pi].mName);
//             tCentiDay d = p.getAvailability(id);
//             if (d>totalDevCentiDaysRemaining) d=totalDevCentiDaysRemaining;
//             if (d>maxindividualrate) d=maxindividualrate;
//             p.decrementAvailability(id,d, z.getFullName());
//             totalDevCentiDaysRemaining-=d;
//             sumCentiDays[pi]+=d;
//         }
//         ++id;
//     }
//     z.mActualEnd = id;

//     double duration = (z.mActualEnd-z.mActualStart).getAsDurationDouble();
//     for (unsigned int pi=0;pi<z.mResources.size();++pi)
//         z.mResources[pi].mLoadingPercent = sumCentiDays[pi]/duration;
// }

void backlog::_dotask_v2_limitedassign(unsigned int itemNdx, const tCentiDay maxAllocation,tCentiDay & remainTeamToday, std::vector<double> & sumCentiDays,tCentiDay & totalDevCentiDaysRemaining, const itemdate id)
{
    backlogitem & z = mItems[itemNdx];
    ASSERT(maxAllocation<=remainTeamToday);
    for (unsigned int pi=0;pi<z.mResources.size();++pi)
    {
        person & p = getPersonByName(z.mResources[pi].mName);
        tCentiDay d = std::min(p.getAvailability(id), maxAllocation);
        if (d>0)
        {
            p.decrementAvailability(id,d, itemNdx);
            totalDevCentiDaysRemaining-=d;
            remainTeamToday-=d;
            sumCentiDays[pi]+=d;
        }
    }
}

void backlog::_dotask_v2(unsigned int itemNdx)
{ //More advanced algorithm that moves forward as fast as possible, but not above average for team to finish on time.
    //    Still march day by day, taking up as much availability as we can for each person,
    //    until we have reached the desired devdays. 
    backlogitem & z = mItems[itemNdx];
    std::vector<double> sumCentiDays(z.mResources.size(),0.0);
    tCentiDay totalDevCentiDaysRemaining = 100 * z.mDevDays;
    itemdate id=z.mActualStart;
    ASSERT(!z.mActualStart.isForever());
    while (totalDevCentiDaysRemaining>0)
    { // loop over days (id)
        unsigned int calDaysPast = (id - z.mActualStart).getAsDurationUInt();
        unsigned int calDaysRemain = 0;
        if (z.mMinCalendarDays>calDaysPast)
            calDaysRemain = z.mMinCalendarDays - calDaysPast; // includes today.

        tCentiDay maxTeamToday = std::min(totalDevCentiDaysRemaining, (tCentiDay)(100*z.mResources.size()));
        if (calDaysRemain>0)
        { // check if we're going too fast to finish in the calendar days remaining.
            tCentiDay maxSpeedRemain = (tCentiDay)((double)totalDevCentiDaysRemaining/(double)calDaysRemain);
            maxTeamToday = std::min(maxTeamToday, maxSpeedRemain);
        }
        tCentiDay remainTeamToday = maxTeamToday;

        // no we need to see if we can allocate maxTeamToday centidays amongst the resources...
        tCentiDay evenPace = maxTeamToday / (z.mResources.size()); // if everyone could go as fast as we like, this is how fast we'd go.

        // go through once limiting to an even pace.
        _dotask_v2_limitedassign(itemNdx,evenPace,remainTeamToday,sumCentiDays,totalDevCentiDaysRemaining,id);
        // now go through again and overallocate (above evenPace) anything left over if we can. Will bias towards first resouce but good enough.
        _dotask_v2_limitedassign(itemNdx,remainTeamToday,remainTeamToday,sumCentiDays,totalDevCentiDaysRemaining,id);

        ++id;
    }
    z.mActualEnd = id;

    double duration = (z.mActualEnd-z.mActualStart).getAsDurationDouble();
    for (unsigned int pi=0;pi<z.mResources.size();++pi)
        z.mResources[pi].mLoadingPercent = sumCentiDays[pi]/duration;
}

// ---------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------------

void backlog::_schedule()
{
    // these two order the tasks optimally.
    _prioritiseAndMergeTeams();     // merge the tasks from each team list together, based on project priorities.
    _topological_sort();            // sort based on task dependency directed graph.

    // now figure out start and end dates for the items, based on human resourcing, reserving resources as we go.
    for (unsigned int zndx=0; zndx<mItems.size();++zndx)
    {
        _determinestart_and_dotask(zndx);
        ASSERT(!mItems[zndx].mActualStart.isForever());
    }
    // printorder(mItems);
}