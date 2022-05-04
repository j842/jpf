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

backlogitem::backlogitem(const std::string s, const unsigned int teamndx, const projects &p) 
{
    std::vector<std::string> items;
    unsigned int numitems;
    if (!simplecsv::splitcsv(s,items,numitems))
        TERMINATE("unable to parse backlog item:"+ s);

    if (numitems!=11)
        TERMINATE("Unexpected number of columns in backlog item: "+s);

    set(items,numitems,teamndx,p);
}

backlogitem::backlogitem(const std::vector<std::string> items, const unsigned int numitems, const unsigned int teamndx, const projects &p)
{
    set(items,numitems,teamndx,p);
}

void backlogitem::set(const std::vector<std::string> items, const unsigned int numitems, const unsigned int teamndx, const projects &p)
{
    mTeamNdx = teamndx;

    ASSERT(items.size()==10);

    // 0 - project
    mProject = p.getIndexByID(items[0]);
    if (mProject==eNotFound) 
        TERMINATE("Could not find project for "+items[0]);
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

    mPriority=0;
    mMergePriority=0;
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
    mTeamsItems.resize(mTeams.size());

    for (unsigned int i=0; i<t.size() ;++i)
        {
            unsigned int j=1;
            std::string filename="team-"+makelower(t[i].mId) + ".csv";
            simplecsv teambacklog(filename,true,10);
            if (!teambacklog.openedOkay())
                TERMINATE("Failed to open team backlog file "+filename);
            std::vector<std::string> items;
            unsigned int numitems;
            while (teambacklog.getline(items,numitems,10))
            {
                backlogitem b(items,numitems,i,p);
                unsigned int n = getItemIndexFromId(b.mId);
                if (n!=eNotFound)
                    TERMINATE(S()<<"Duplicate ID " << b.mId<<" of two backlog items:"<<std::endl<<b.mDescription<<std::endl<<mItems[n].mDescription);

                mTeamsItems[b.mTeamNdx].push_back( b );
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


person & backlog::getPersonByName(const std::string name) 
{   
    for (auto & p : mPeople)
        if (iSame(name,p.mName))
            return p;

    TERMINATE( "Unable to locate resource " + name + " in any team.");
    return mPeople[0];
}

unsigned int backlog::getPersonIndexFromName(const std::string name) const
{
    for (unsigned int i=0;i<mPeople.size();++i)
        if (iSame(mPeople[i].mName, name)) return i;
    TERMINATE("Unable to find name "+name);
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
    std::cout << "Scheduling backlog items..."<<std::endl;

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



void backlog::_calc_project_summary()
{ // scheduling done.
    for (auto & z : mProjects)
    {
        z.mTotalDevDays=0.0;
        z.mActualStart.setForever();
        z.mActualEnd.setToStart();
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

    for (auto & proj : mProjects)
        if (proj.mActualStart.isForever()) // no tasks.
            proj.mActualStart.setToStart();
}
