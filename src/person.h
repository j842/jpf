#ifndef __PERSON_H
#define __PERSON_H

#include <string>
#include <vector>

#include "itemdate.h"
#include "teams.h"
#include "publicholidays.h"


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
        intervals(const intervals & other);
 
        itemdate getEarliestStart(itemdate fromstart) const;                
        tCentiDay getMaxAvialability() const {return mMaxAvailability;}
        tCentiDay getAvailability(itemdate day) const;

        void decrementAvailability(itemdate day, tCentiDay decrement, unsigned int itemNdx);
        void registerHoliday(daterange dr);
        void getChunks(unsigned int day, std::vector<daychunk> & chunks);

        private:
            void _decrementAvailability(itemdate day, tCentiDay decrement); // does not assign workchunk.

        private:
            tCentiDay mMaxAvailability;
            std::vector<tCentiDay>              mRemainingAvailability; // Centidays. Index is days since start.
            std::vector<std::vector<daychunk>>  mWorkChunks; // First index is days since start.
};

class person : public teammember
{
    public:
        person(const teammember & m, const publicholidays &pubh);

        itemdate getEarliestStart(itemdate fromstart);
        tCentiDay getMaxAvialability() const;
        tCentiDay getAvailability(itemdate day) const;
        void decrementAvailability(itemdate day, tCentiDay decrement,unsigned int itemNdx);

        void getChunks(unsigned int day, const std::vector<daychunk> & chunks);

    private:
        void _registerHolidayString(std::string s);
        void _registerHoliday(daterange dr);

    private:
        intervals mIntervals;

};

typedef std::vector<person> tPersonVec;
class people : public tPersonVec
{
    public:
        people();

        unsigned int getMaxNameWidth() const;
};

#endif