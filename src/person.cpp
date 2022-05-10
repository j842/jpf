#include <climits>
#include <iomanip>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "person.h"
#include "utils.h"
#include "simplecsv.h"
#include "itemdate.h"

intervals::intervals(tCentiDay max_aviailability) : mMaxAvailability(max_aviailability)
{
    ASSERT(mMaxAvailability>=0);
}

intervals::intervals(const intervals & other) : 
    mMaxAvailability(other.getMaxAvialability()),
    mRemainingAvailability(other.mRemainingAvailability)
{
}

itemdate intervals::getEarliestStart(itemdate fromstart) const
{       
    itemdate es(fromstart);

    if (mMaxAvailability==0)
        es.setForever();
    else
        while (getAvailability(es)==0)
            es+=1;

    return es;
}

tCentiDay intervals::getAvailability(itemdate day) const
{
    unsigned int uDay = day.getDayAsIndex();
    if (uDay < mRemainingAvailability.size())
        return mRemainingAvailability[uDay];
    return mMaxAvailability;
}

void intervals::decrementAvailability(itemdate day, tCentiDay decrement)
{
    unsigned int uDay = day.getDayAsIndex();

    if (uDay>=mRemainingAvailability.size())
        mRemainingAvailability.resize(uDay+20, mMaxAvailability);

    ASSERT(mRemainingAvailability[uDay]>=decrement);
    mRemainingAvailability[uDay] = mRemainingAvailability[uDay] - decrement;
}

person::person(const member & m) :
    member(m.mName,m.mEFTProject,m.mEFTBAU,m.mEFTOverhead,m.mLeave,m.mOriginalLeave),
    mIntervals(m.mEFTProject)
{
    if (mLeave.length()>0)
    {
        std::vector<std::string> items;
        bool okay = simplecsv::splitcsv(mLeave, items);
        if (!okay) TERMINATE("Couldn't parse leave for "+mName+" -- "+mLeave);

        for (auto & x : items)
        { // parse leave string. Could be date, or date-date (inclusive).

            std::vector<std::string> strs;
            boost::split(strs, x, boost::is_any_of("-"));
            if (strs.size()==1) strs.push_back(strs[0]);
            ASSERT(strs.size()==2);
            itemdate ds;
            ds.setclip(strs[0]);
            itemdate de;
            de.setclip(strs[1]);
            registerHoliday(ds,de+1);// leave dates are inclusive, make open interval.
        }
    }
}


itemdate person::getEarliestStart(itemdate fromstart)
{
    return mIntervals.getEarliestStart(fromstart);
}

void person::registerHoliday(itemdate start, itemdate end)
{ // end is open interval.
    for (itemdate i=start;i<end;++i)
        decrementAvailability(i, mIntervals.getAvailability(i), "Leave");
}

tCentiDay person::getMaxAvialability() const
{
    return mIntervals.getMaxAvialability();
}

tCentiDay person::getAvailability(itemdate day) const
{
    return mIntervals.getAvailability(day);
}

void person::decrementAvailability(itemdate day, tCentiDay decrement, const std::string & task)
{
    mIntervals.decrementAvailability(day, decrement);
}
