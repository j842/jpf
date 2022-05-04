#include <algorithm>
#include <cctype>
#include <string>
#include <iomanip>  
#include <sstream>
#include <numeric>

#include "backlog.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

backlogitem::backlogitem(const std::string s, const unsigned int teamndx, const projects &p,unsigned int priority) 
{
    std::vector<std::string> items;
    unsigned int numitems;
    if (!simplecsv::splitcsv(s,items,numitems))
        terminate("unable to parse backlog item:"+ s);

    if (numitems!=11)
        terminate("Unexpected number of columns in backlog item: "+s);

    set(items,numitems,teamndx,p,priority);
}

backlogitem::backlogitem(const std::vector<std::string> items, const unsigned int numitems, const unsigned int teamndx, const projects &p,unsigned int priority)
{
    set(items,numitems,teamndx,p,priority);
}

void backlogitem::set(const std::vector<std::string> items, const unsigned int numitems, const unsigned int teamndx, const projects &p,unsigned int priority)
{
    mTeamNdx = teamndx;

    ASSERT(items.size()==10);

    // 0 - project
    mProject = p.getIndexByID(items[0]);
    if (mProject==eNotFound) 
        terminate("Could not find project for "+items[0]);
    mProjectName = p[mProject].getId();

    // 1 - ID
    mId=items[1];
    // 2 - Description
    mDescription=items[2];
    // 3 - Min Calendar Workdays
    mMinCalendarDays=atoi(items[3].c_str());
    // 4 - Min Dev Days (TOTAL)
    mDevDays=atoi(items[4].c_str());
    // 5 - Earliest Start Date
    mEarliestStart.set(items[5]);

    // 6 - blocking resources
    unsigned int x;
    std::vector<std::string> names;
    simplecsv::splitcsv(items[6],names,x);
    for (auto & name : names)
        mResources.push_back( resource(name, true) ); // blocking resources

    // 7 - contributing resources
    names.clear();
    simplecsv::splitcsv(items[7],names,x);
    for (auto & name : names)
        mResources.push_back( resource(name , false) ); // contributing resources

    // 8 - dependencies
    simplecsv::splitcsv(items[8],mDependencies,x);

    // 9 - comments (not stored)

    mPriority=priority;
    mOriginalPriority=priority;
}


bool backlogitem::hasDependency(std::string d)
{
    for (auto & ch : mDependencies)
        if (iSame(ch,d))
            return true;

    return false;
}
itemdate backlogitem::getDuration() const 
{ 
    return mActualEnd-mActualStart;
}

std::string backlogitem::getFullName() const
{
    return mProjectName + " - " + mDescription;
}



std::string makelower(const std::string & s)
{
    std::string slower(s);
    std::transform(slower.begin(), slower.end(), slower.begin(),
        [](unsigned char c){ return std::tolower(c); });

    return slower;
}




backlog::backlog(projects & p, const teams & t) : mTeams(t), mProjects(p)
{
    for (unsigned int i=0; i<t.size() ;++i)
        {
            unsigned int j=1;
            std::string filename="team-"+makelower(t[i].mId) + ".csv";
            simplecsv teambacklog(filename,true,10);
            if (!teambacklog.openedOkay())
                terminate("Failed to open team backlog file "+filename);
            std::vector<std::string> items;
            unsigned int numitems;
            while (teambacklog.getline(items,numitems,10))
            {
                backlogitem b(items,numitems,i,p,j);
                unsigned int n = getItemIndexFromId(b.mId);
                if (n!=eNotFound)
                    terminate(S()<<"Duplicate ID " << b.mId<<" of two backlog items:"<<std::endl<<b.mDescription<<std::endl<<mItems[n].mDescription);

                mItems.push_back( b );
                ++j;
            }
        }

    // copy people name/maxtime from the teams list into our people list.
    for (const auto & x : mTeams)
        for (const auto & y : x.mMembers)
            mPeople.push_back( person( y ));
}


void backlog::prioritySortArray( std::vector<int> & v ) const
{
    // sort by project priority to make a little more readable. :-) 
    v.resize(mItems.size());
    std::iota(v.begin(), v.end(), 0);
    std::sort(begin(v), end(v), [this](int index_left, int index_right) { 
        if (mItems[index_left].mProject != mItems[index_right].mProject)
            return mItems[index_left].mProject < mItems[index_right].mProject; 
        else
            if (mItems[index_left].mActualEnd != mItems[index_right].mActualEnd)
                return mItems[index_left].mActualEnd < mItems[index_right].mActualEnd;
            else
            {
                if (mItems[index_left].mDevDays==0) return false;
                if (mItems[index_right].mDevDays==0) return true; 
                return index_left<index_right;
            }
        });//Ascending order.
}


void vmove(std::vector<backlogitem> & v, const std::size_t i_old, const std::size_t i_new)
{
    auto it = v.begin();
    std::rotate( it + i_new, it + i_old, it + i_old + 1);
}

unsigned int backlog::getItemIndexFromId(const std::string id) const
{
    for (unsigned int k=0;k<mItems.size();++k)
        if (iSame(mItems[k].mId,id) )
            return k;

    return eNotFound;
}


// Compares two items by priority
bool comparePriority(const backlogitem & i1, const backlogitem & i2)
{
    return (i1.mPriority < i2.mPriority);
}


person & backlog::getPersonByName(const std::string name) 
{   
    for (auto & p : mPeople)
        if (iSame(name,p.mName))
            return p;

    terminate( "Unable to locate resource " + name + " in any team.");
    return mPeople[0];
}

unsigned int backlog::getPersonIndexFromName(const std::string name) const
{
    for (unsigned int i=0;i<mPeople.size();++i)
        if (iSame(mPeople[i].mName, name)) return i;
    terminate("Unable to find name "+name);
    return 0;
}



std::string quoted(std::string s)
{
    return std::string("\"") + s + std::string("\"");
}



// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------



void backlog::schedule()
{
    std::cout << "Scheduling "<<mItems.size()<<" backlog items..."<<std::endl;

    timer t;

    _schedule();
    _calc_project_summary();

    std::cout << "Scheduling done in " << std::setprecision(3) << t.stop() << " ms." << std::endl;
}

// -----------------------------------------------------------------------------------------------------------------

void printorder(const std::vector<backlogitem> & v)
{
    for (auto & z : v) std::cout<<z.mId<<" ";
    std::cout<<std::endl;
}

void backlog::_topological_visit(int node, std::vector<bool> & tempMarks, std::vector<bool> & permMarks, std::vector<unsigned int> & scheduledList)
{
    if (permMarks[node]) return;
    if (tempMarks[node]) terminate("Not a directed acyclic graph - circular dependency involving "+mItems[node].mId);

    tempMarks[node]=true;

    std::vector<int> depNdxs;
    for (auto & d : mItems[node].mDependencies)
        {
            unsigned int k = getItemIndexFromId(d);
            if (k==eNotFound) terminate( S() << "The dependency \""<< d <<"\" does not exist!");
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

void backlog::_schedule()
{
    // first sort by priorty.
    std::sort( mItems.begin(), mItems.end(), comparePriority);
    // printorder(mItems);

    _topological_sort(); // sort based on dependency directed graph.

    // now figure out star and end dates for the items, reserving resources as we go.
    for (unsigned int zndx=0; zndx<mItems.size();++zndx)
    {
        auto & z = mItems[zndx];
        // set start date based on EarliestStart
        z.mActualStart = z.mEarliestStart;

        // now update based on dependencies
        for (unsigned int j=0;j<z.mDependencies.size();j++)
        {
            unsigned int k = getItemIndexFromId(z.mDependencies[j]);
            if (k==eNotFound) terminate("programming error - couldn't find already found dependency "+z.mDependencies[j]);
            if (k>zndx)
                terminate(S() << "Out of order dependency : "<<z.mDependencies[j]<<" is needed for "
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
                            terminate(p.mName+" is blocking but can never start task "+z.getFullName());                         

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
        }
    }

    // printorder(mItems);
}



void backlog::_calc_project_summary()
{ // scheduling done.
    for (auto & z : mProjects)
    {
        z.mTotalDevDays=0.0;
        z.mActualStart.setForever();
        z.mActualEnd=0;
    }
    
    // iterate through tasks, taking max and min duration.
    for (const auto & task : mItems)
    {
        auto & p = mProjects[task.mProject];

        if (task.mActualStart < p.mActualStart)
            p.mActualStart=task.mActualStart;

        if (task.mActualEnd > p.mActualEnd)
            p.mActualEnd=task.mActualEnd;

        if (task.mDependencies.size()==0)
            p.mTotalDevDays += task.mDevDays;
        else
            for (auto & x : task.mResources)
                p.mTotalDevDays += task.getDuration().getAsDurationDouble() *x.mLoadingPercent/100.0;
    }
}
