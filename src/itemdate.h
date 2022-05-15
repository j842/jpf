#ifndef __ITEMDATE_H
#define __ITEMDATE_H


#include <string>
#include <boost/date_time.hpp>
#include "utils.h"
#include <cppunit/extensions/HelperMacros.h>

// a centiday is 1/100th of a day.
class tCentiDay
{
    public:
        tCentiDay() : mD(0) {}
        tCentiDay(unsigned long x) : mD(x) {}
        unsigned long getL() const {return mD;}
        unsigned long getRoundUpDays() const { return (unsigned long)((99.0 + mD)/100.0); }

        explicit operator double() const { return mD; }
        operator unsigned long() const { return mD; }

        tCentiDay& operator+=(const tCentiDay& rhs) { mD+=rhs.getL(); return *this;}
        tCentiDay& operator-=(const tCentiDay& rhs) { ASSERT(rhs.getL()<=mD); mD-=rhs.getL(); return *this;}
    private:
        unsigned long mD;
};

class monthIndex;

class simpledate
{
    public:
        simpledate();
        simpledate(std::string datestr);  // set the date based on a dd/mm/yy string.
        simpledate(const boost::gregorian::date & d);

        std::string getStr() const;     
        std::string getStrGantt() const;
        std::string getStr_short() const;     
        std::string getStr_nice_long() const;
        std::string getStr_nice_short() const;
        std::string getAsGoogleNewDate() const;

        void setForever();
        void setToStart();
        bool isForever() const;

        boost::gregorian::date getGregorian() const;
        monthIndex getMonthIndex() const;
        
    public:
        static simpledate parseDateStringDDMMYY(std::string datestr);

        static simpledate nextWorkDay(simpledate d);
        static unsigned long countWorkDays(simpledate dA, simpledate dB); // half open interval.

        static simpledate WorkDays2Date(unsigned long ndays);
        static unsigned long Date2WorkDays(simpledate d0);

    public:
        friend bool operator==(const simpledate& lhs, const simpledate & rhs) { return (lhs.getGregorian()==rhs.getGregorian()); }
        friend bool operator!=(const simpledate& lhs, const simpledate & rhs) { return !(lhs==rhs); }
        friend bool operator<(const simpledate& lhs, const simpledate & rhs) { return lhs.getGregorian()<rhs.getGregorian(); }
        friend bool operator>(const simpledate& lhs, const simpledate & rhs) { return lhs.getGregorian()>rhs.getGregorian(); }
        friend bool operator>=(const simpledate& lhs, const simpledate & rhs) { return lhs.getGregorian()>=rhs.getGregorian(); }
        friend bool operator<=(const simpledate& lhs, const simpledate & rhs) { return lhs.getGregorian()<=rhs.getGregorian(); }

    protected:
        std::string _getstr(const std::locale & fmt) const;

    protected:
        boost::gregorian::date mD;
};

unsigned long wdduration(const simpledate& istart, const simpledate& iend); // difference in work days


class simpledate_test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( simpledate_test );
    CPPUNIT_TEST( simpledate_test0 );
    CPPUNIT_TEST( simpledate_test1 );
    CPPUNIT_TEST( simpledate_test2 );
    CPPUNIT_TEST( simpledate_test3 );
    CPPUNIT_TEST( simpledate_test4 );
    CPPUNIT_TEST_SUITE_END();

    public:
        void simpledate_test0();
        void simpledate_test1();
        void simpledate_test2();
        void simpledate_test3();
        void simpledate_test4();
};


class monthIndex
{
    public:
        monthIndex(unsigned long n) : mN(n) {}
        monthIndex(simpledate d); 

        boost::gregorian::date getFirstMonthDay() const;
        boost::gregorian::date getLastMonthDay() const;

        operator unsigned long() const {return mN;}

        std::string getString() const;

        unsigned long workingDaysInMonth() const;

    private:
        unsigned long mN;
};

// simple class for coordinate transformation from calendar date (local timezone) to number of working days from today.
// can only hold dates that are workdays (not weekends).
class itemdate : public simpledate
{
    public:
        itemdate();                     // set the date as today.
        itemdate(std::string datestr);  // set the date based on a dd/mm/yy string.
        itemdate(const itemdate &obj);  // copy ctor
        itemdate(const boost::gregorian::date & d);
        itemdate(unsigned long dayIndex);

        bool setclip(std::string datestr);  // set to the next workday after start from a dd/mm/yy string. 
        bool set(const boost::gregorian::date & d);  // set to the next workday from a gregorian date.
        void decrement();
        void increment();
        unsigned long getDayAsIndex() const;


    public:
        friend itemdate operator+(const itemdate& lhs, unsigned long rhs); // advance x work days.
};

typedef enum
{
    kHalfOpenInterval,
    kClosedInterval

} tIntervalTypes;

class daterange
{
    public:
        daterange(std::string s, tIntervalTypes t); 

        itemdate getStart() const;
        itemdate getEnd() const;    // half open interval (end is not in the interval)

        std::string getRangeAsString() const;

        void setStart(itemdate start);
        void setEnd(itemdate end, tIntervalTypes t);

        private:
            itemdate mStart;
            itemdate mEnd; // half open interval!
};


#endif