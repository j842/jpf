#include <climits>
#include <iomanip>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "scheduler_person.h"
#include "utils.h"
#include "simplecsv.h"
#include "itemdate.h"

namespace scheduler
{

    daychunk::daychunk(unsigned int itemNdx, tCentiDay effort) : mItemIndex(itemNdx), mEffort(effort)
    {
    }

    intervals::intervals(tCentiDay max_aviailability) : mMaxAvailability(max_aviailability)
    {
        ASSERT(mMaxAvailability >= 0);
    }

    intervals::intervals(const intervals &other) : mMaxAvailability(other.getMaxAvialability()),
                                                   mRemainingAvailability(other.mRemainingAvailability)
    {
    }

    itemdate intervals::getEarliestStart(itemdate fromstart) const
    {
        itemdate es(fromstart);

        if (mMaxAvailability == 0)
            es.setForever();
        else
            while (getAvailability(es) == 0)
                es += 1;

        return es;
    }

    tCentiDay intervals::getAvailability(itemdate day) const
    {
        unsigned int uDay = day.getDayAsIndex();
        if (uDay < mRemainingAvailability.size())
            return mRemainingAvailability[uDay];
        return mMaxAvailability;
    }

    void intervals::_decrementAvailability(itemdate day, tCentiDay decrement) // does not assign workchunk.
    {
        unsigned int uDay = day.getDayAsIndex();

        if (uDay >= mRemainingAvailability.size())
            mRemainingAvailability.resize(uDay + 20, mMaxAvailability);

        ASSERT(mRemainingAvailability[uDay] >= decrement);
        mRemainingAvailability[uDay] = mRemainingAvailability[uDay] - decrement;
    }

    void intervals::decrementAvailability(itemdate day, tCentiDay decrement, unsigned int itemNdx)
    {
        _decrementAvailability(day, decrement);

        unsigned int uDay = day.getDayAsIndex();
        if (uDay >= mWorkChunks.size())
            mWorkChunks.resize(uDay + 20);

        daychunk dc(itemNdx, decrement);
        mWorkChunks[uDay].push_back(dc);
    }

    void intervals::registerHoliday(daterange dr)
    { // dr is half open interval, so we don't include the end.
        for (itemdate i = dr.getStart(); i < dr.getEnd(); ++i)
            _decrementAvailability(i, getAvailability(i));
    }

    const std::vector<daychunk> &intervals::getChunks(unsigned int day)
    {
        if (day >= mWorkChunks.size())
            mWorkChunks.resize(day + 20);

        ASSERT(day >= 0 && day < mWorkChunks.size());
        return mWorkChunks[day];
    }

    // ------------------------------------------------------------------------------------------------------------

    person::person(const teammember &m, const inputfiles::publicholidays &pubh) : teammember(m.mName, m.mEFTProject, m.mEFTBAU, m.mEFTOverhead, m.getLeave()),
                                                                      mIntervals(m.mEFTProject)
    {
        _registerHolidayString(getLeave());
        _registerHolidayString(pubh.getStr());
    }

    void person::_registerHolidayString(std::string s)
    {
        if (s.length() > 0)
        {
            std::vector<std::string> items;
            bool okay = simplecsv::splitcsv(s, items);
            if (!okay)
                TERMINATE("Couldn't parse leave for " + mName + " -- " + s);

            for (auto &x : items)
            { // parse leave string. Could be date, or date-date (inclusive). Closed interval.
                daterange dr(x, kClosedInterval);
                _registerHoliday(dr);
            }
        }
    }

    itemdate person::getEarliestStart(itemdate fromstart)
    {
        return mIntervals.getEarliestStart(fromstart);
    }

    void person::_registerHoliday(daterange dr)
    { // end is open interval.
        mIntervals.registerHoliday(dr);
    }

    tCentiDay person::getMaxAvialability() const
    {
        return mIntervals.getMaxAvialability();
    }

    tCentiDay person::getAvailability(itemdate day) const
    {
        return mIntervals.getAvailability(day);
    }

    void person::decrementAvailability(itemdate day, tCentiDay decrement, unsigned int itemNdx)
    {
        mIntervals.decrementAvailability(day, decrement, itemNdx);
    }

    const std::vector<daychunk> &person::getChunks(unsigned int day)
    {
        return mIntervals.getChunks(day);
    }

    // ------------------------------------------------------------------------------------------------------------

    people::people() : tPersonVec()
    {
    }

    unsigned int people::getMaxNameWidth() const
    {
        unsigned int lpn = 0;
        for (auto &i : *this)
            lpn = std::max(lpn, (unsigned int)i.mName.length());
        return lpn;
    }

} // namespace