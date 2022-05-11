#ifndef __PERSON_H
#define __PERSON_H

#include <string>
#include <vector>

#include "itemdate.h"
#include "teams.h"

// // half open interval!!
// class interval
// {
//     public:
//         interval(int start,int end,double avail) : mStart(start),mEnd(end),mAvailability(avail) 
//         {}

//         int fulldays() const {return mEnd-mStart;}

//         //double get_total_interval_availability() const {return mAvailability * (mEnd-mStart);}

//         int mStart;     // half open - interval does include the start day.
//         int mEnd;       // half open - interval does _not_ include the end day!
//         double mAvailability;

//         std::vector<std::string> mTasks;
// };

// store a persons remaining availability.
class intervals
{
    public:
        intervals(tCentiDay max_aviailability);
        intervals(const intervals & other);
 
        itemdate getEarliestStart(itemdate fromstart) const;                
        tCentiDay getMaxAvialability() const {return mMaxAvailability;}
        tCentiDay getAvailability(itemdate day) const;

        void decrementAvailability(itemdate day, tCentiDay decrement);

        private:
            tCentiDay mMaxAvailability;
            std::vector<tCentiDay> mRemainingAvailability; // Centidays.
};

class person : public member
{
    public:
        person(const member & m);

        itemdate getEarliestStart(itemdate fromstart);
        tCentiDay getMaxAvialability() const;
        tCentiDay getAvailability(itemdate day) const;
        void decrementAvailability(itemdate day, tCentiDay decrement,const std::string & task);
        void registerHoliday(daterange dr);

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