#ifndef __PERSON_H
#define __PERSON_H

#include <string>
#include <vector>

#include "workdate.h"
#include "inputfiles_teams.h"
#include "inputfiles_publicholidays.h"

namespace scheduler
{

    class daychunk
    { // most basic component of work.
    public:
        daychunk(unsigned int itemNdx, tCentiDay effort);
        unsigned int mItemIndex; // backlog item.
        const tCentiDay mEffort;
    };

    // store a persons remaining availability.
    class intervals
    {
    public:
        intervals(tCentiDay max_aviailability);
        intervals(const intervals &other);

        workdate getEarliestStart(workdate fromstart) const;
        tCentiDay getMaxAvialability() const { return mMaxAvailability; }
        tCentiDay getAvailability(workdate day) const;

        void decrementAvailability(unsigned long uDay, tCentiDay decrement, unsigned int itemNdx);
        void registerHoliday(leaverange dr);

        const std::vector<daychunk> &getChunks(unsigned int day) const;
        unsigned long numChunkDays() const;
        
    private:
        void _decrementAvailability(unsigned long uDay, tCentiDay decrement); // does not assign workchunk.

    private:
        tCentiDay mMaxAvailability;
        std::vector<tCentiDay> mRemainingAvailability;  // Centidays. Index is days since start.
        std::vector<std::vector<daychunk>> mWorkChunks; // First index is days since start.
        std::vector<daychunk> mEmptyChunk;
    };

    class scheduledperson : public inputfiles::teammember
    {
    public:
        scheduledperson(const inputfiles::teammember &m, const inputfiles::publicholidays &pubh);

        workdate getEarliestStart(workdate fromstart);
        tCentiDay getMaxAvialability() const;
        tCentiDay getAvailability(workdate day) const;
        void decrementAvailability(unsigned long uDay, tCentiDay decrement, unsigned int itemNdx);

        const std::vector<daychunk> &getChunks(unsigned int day) const;
        unsigned long numChunkDays() const;

        unsigned long holidaysInMonth(unsigned long month) const;

    private:
        void _registerHoliday(leaverange dr);

    private:
        intervals mIntervals;
        std::vector<leaverange> mHolidays;
    };

    typedef std::vector<scheduledperson> tPersonVec;
    class scheduledpeople : public tPersonVec
    {
    public:
        scheduledpeople();

        unsigned int getMaxNameWidth() const;
    };

} // namesapce

#endif