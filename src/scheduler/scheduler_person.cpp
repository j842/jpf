#include <climits>
#include <iomanip>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "scheduler_person.h"
#include "utils.h"
#include "simplecsv.h"
#include "workdate.h"

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

    workdate intervals::getEarliestStart(workdate fromstart) const
    {
        workdate es(fromstart);

        if (mMaxAvailability == 0)
            es.setForever();
        else
            while (getAvailability(es) == 0)
                es.incrementWorkDay();
                
        return es;
    }

    tCentiDay intervals::getAvailability(workdate day) const
    {
        unsigned int uDay = day.getDayAsIndex();
        if (uDay < mRemainingAvailability.size())
            return mRemainingAvailability[uDay];
        return mMaxAvailability;
    }

    void intervals::_decrementAvailability(unsigned long uDay, tCentiDay decrement) // does not assign workchunk.
    {
        if (uDay >= mRemainingAvailability.size())
            mRemainingAvailability.resize(uDay + 20, mMaxAvailability);

        ASSERT(mRemainingAvailability[uDay] >= decrement);
        mRemainingAvailability[uDay] = mRemainingAvailability[uDay] - decrement;
    }

    void intervals::decrementAvailability(unsigned long uDay, tCentiDay decrement, unsigned int itemNdx)
    {
        _decrementAvailability(uDay, decrement);

        if (uDay >= mWorkChunks.size())
            mWorkChunks.resize(uDay + 20);

        daychunk dc(itemNdx, decrement);
        mWorkChunks[uDay].push_back(dc);
    }

    void intervals::registerHoliday(leaverange dr)
    { // dr is half open interval, so we don't include the end.
        if (!dr.isEmpty())
            for (unsigned long i = dr.getStartasIndex(); i < dr.getEndasIndex(); ++i) // half open.
                _decrementAvailability(i, getAvailability(i));
    }

    const std::vector<daychunk> &intervals::getChunks(unsigned int day) const
    {
        if (day >= mWorkChunks.size())
            return mEmptyChunk;

        return mWorkChunks[day];
    }

    unsigned long intervals::numChunkDays() const
    {
        return mWorkChunks.size();
    }


    // ------------------------------------------------------------------------------------------------------------

    scheduledperson::scheduledperson(const teammember &m, const inputfiles::publicholidays &pubh) : teammember(m.mName, m.mEFTProject, m.mEFTBAU, m.mEFTOverhead, m.getLeave()),
                                                                      mIntervals(m.mEFTProject)
    {
        for (auto & i : getLeave())
            _registerHoliday(i);

        for (auto & i : pubh.getHolidays())
            _registerHoliday(i);
    }

    void scheduledperson::_registerHolidayString(std::string s)
    {
        if (s.length() > 0)
        {
            std::vector<std::string> items;
            bool okay = simplecsv::splitcsv(s, items);
            if (!okay)
                TERMINATE("Couldn't parse leave for " + mName + " -- " + s);

            for (auto &x : items) // parse leave string. Could be date, or date-date (inclusive). Closed interval.
                _registerHoliday(leaverange(x));
        }
    }

    workdate scheduledperson::getEarliestStart(workdate fromstart)
    {
        return mIntervals.getEarliestStart(fromstart);
    }

    void scheduledperson::_registerHoliday(leaverange dr)
    { // end is open interval.
        if (!dr.isEmpty())
        {
            mIntervals.registerHoliday(dr);
            mHolidays.push_back(dr);
        }
    }

    tCentiDay scheduledperson::getMaxAvialability() const
    {
        return mIntervals.getMaxAvialability();
    }

    tCentiDay scheduledperson::getAvailability(workdate day) const
    {
        return mIntervals.getAvailability(day);
    }

    void scheduledperson::decrementAvailability(unsigned long uDay, tCentiDay decrement, unsigned int itemNdx)
    {
        mIntervals.decrementAvailability(uDay, decrement, itemNdx);
    }

    const std::vector<daychunk> &scheduledperson::getChunks(unsigned int day) const
    {
        return mIntervals.getChunks(day);
    }
    unsigned long scheduledperson::numChunkDays() const
    {
        return mIntervals.numChunkDays();
    }

    unsigned long scheduledperson::holidaysInMonth(unsigned long month) const
    {
        unsigned long tally=0;
        for (auto & dr : mHolidays)
            tally += dr.holidayDaysInMonth(month);
        return tally;
    }


    // ------------------------------------------------------------------------------------------------------------

    scheduledpeople::scheduledpeople() : tPersonVec()
    {
    }

    unsigned int scheduledpeople::getMaxNameWidth() const
    {
        unsigned int lpn = 0;
        for (auto &i : *this)
            lpn = std::max(lpn, (unsigned int)i.mName.length());
        return lpn;
    }

} // namespace