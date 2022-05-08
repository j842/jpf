#include "backlogitem.h"

#include "simplecsv.h"

backlogitem::backlogitem(const std::string s, const unsigned int teamndx, const projects &p) 
{
    std::vector<std::string> csvitems;
    if (!simplecsv::splitcsv(s,csvitems))
        TERMINATE("unable to parse backlog item:"+ s);

    if (csvitems.size()!=11)
        TERMINATE("Unexpected number of columns in backlog item: "+s);

    set(csvitems,teamndx,p);
}

backlogitem::backlogitem(const std::vector<std::string> csvitems, const unsigned int teamndx, const projects &p)
{
    set(csvitems,teamndx,p);
}

void backlogitem::set(const std::vector<std::string> csvitems, const unsigned int teamndx, const projects &p)
{
    mTeamNdx = teamndx;

    ASSERT(csvitems.size()==10);

    // 0 - project
    mProject = p.getIndexByID(csvitems[0]);
    if (mProject==eNotFound) 
        TERMINATE("Could not find project for "+csvitems[0]);
    mProjectName = p[mProject].getId();

    // 1 - ID
    mId=csvitems[1];
    // 2 - Description
    mDescription=csvitems[2];
    // 3 - Min Calendar Workdays
    mMinCalendarDays=atoi(csvitems[3].c_str());
    // 4 - Min Dev Days (TOTAL)
    mDevDays=atoi(csvitems[4].c_str());
    // 5 - Earliest Start Date
    mEarliestStart.set(csvitems[5]);

    // 6 - blocking resources
    std::vector<std::string> names;
    simplecsv::splitcsv(csvitems[6],names);
    for (auto & name : names)
        mResources.push_back( resource(name, true) ); // blocking resources

    // 7 - contributing resources
    names.clear();
    simplecsv::splitcsv(csvitems[7],names);
    for (auto & name : names)
        mResources.push_back( resource(name , false) ); // contributing resources

    // 8 - dependencies
    simplecsv::splitcsv(csvitems[8],mDependencies);

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


