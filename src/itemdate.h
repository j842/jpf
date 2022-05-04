#ifndef __ITEMDATE_H
#define __ITEMDATE_H


#include <string>
#include <boost/date_time.hpp>
#include "utils.h"

// simple class for coordinate transformation from calendar date (local timezone) to number of working days from today.
class itemdate
{
    public:
        itemdate();                     // set the date as today.
        itemdate(std::string datestr);  // set the date based on a dd/mm/yy string.
        itemdate(const itemdate &obj);  // copy ctor
        itemdate(const boost::gregorian::date & d);
        itemdate(unsigned int dayIndex);

        bool set(std::string datestr);  // set the date based on a dd/mm/yy string.
        bool setclip(std::string datestr);  // set the date based on a dd/mm/yy string. If < start, det to start.
        bool set(const boost::gregorian::date & d);  // set the date based on a dd/mm/yy string.
        void decrement();
        unsigned int getDayAsIndex() const;

        std::string getStr() const;     // get the actual date as a string corresponding to mDay workdays from today.
        std::string getStrGantt() const;
        std::string getStr_short() const;     // get the actual date as a string corresponding to mDay workdays from today.

        friend itemdate operator+(const itemdate& lhs,const int& rhs) { return itemdate(lhs.mDay+rhs);}
        friend itemdate operator-(const itemdate& lhs,const int& rhs) { return itemdate(lhs.mDay-rhs);}
        friend itemdate operator+(const itemdate& lhs,const itemdate& rhs) { return itemdate(lhs.mDay+rhs.mDay);}
        friend itemdate operator-(const itemdate& lhs,const itemdate& rhs) { return itemdate(lhs.mDay-rhs.mDay);}

        itemdate& operator+=(const itemdate& rhs) { mDay+=rhs.mDay; return *this;}
        itemdate& operator-=(const itemdate& rhs) { ASSERT(rhs.mDay>=mDay); mDay-=rhs.mDay; return *this;}
        itemdate& operator+=(const unsigned int& rhs) { mDay+=rhs; return *this;}
        itemdate& operator-=(const unsigned int& rhs) { ASSERT(rhs>=mDay); mDay-=rhs; return *this;}
        itemdate& operator++() { mDay++; return *this;}
 
        friend bool operator< (const itemdate& lhs, const itemdate& rhs){ return lhs.mDay < rhs.mDay;}
        friend bool operator> (const itemdate& lhs, const itemdate& rhs){ return lhs.mDay > rhs.mDay; }
        friend bool operator<=(const itemdate& lhs, const itemdate& rhs){ return lhs.mDay <= rhs.mDay; }
        friend bool operator>=(const itemdate& lhs, const itemdate& rhs){ return lhs.mDay >= rhs.mDay; }

        friend bool operator==(const itemdate& lhs, const itemdate& rhs){ return lhs.mDay==rhs.mDay; }
        friend bool operator!=(const itemdate& lhs, const itemdate& rhs){ return lhs.mDay!=rhs.mDay; }

        friend itemdate itemdate_max(const itemdate& lhs, const itemdate& rhs) { return lhs.mDay>rhs.mDay ? lhs : rhs; }

        static boost::gregorian::date Today();

        static boost::gregorian::date nextWorkday(boost::gregorian::date d0);
        static boost::gregorian::date parseDateStringDDMMYY(std::string datestr);
        static std::string date2strNice(boost::gregorian::date d0);
        static boost::gregorian::date firstdayofmonth(unsigned int monthFromStart);
        static boost::gregorian::date lastdayofmonth(unsigned int monthFromStart);

        static std::string getMonthAsString(unsigned int monthFromStart);
        static unsigned int getEndMonth();

        unsigned int getMonthFromStart() const;
        static unsigned int getMonthFromStart(boost::gregorian::date d1);
        std::string getMonthAsString() const;

        std::string getAsGoogleNewDate() const;

        void setForever();
        bool isForever() const;

        std::string getAsDurationString() const;
        double getAsDurationDouble() const;

        private:
            unsigned int mDay; // number of workdays from today (today = 0, weekends don't count)

            std::string getstr(const std::locale & fmt) const;
            
            long countWeekDays(boost::gregorian::date d0, boost::gregorian::date d1) const;
            boost::gregorian::date day2date(unsigned int day) const;
};


#endif