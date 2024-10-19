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
        void setInf() {mD=ULONG_MAX;}
        bool isInf() const {return mD==ULONG_MAX;}

        explicit operator double() const { return mD; }
        operator unsigned long() const { return mD; }

        tCentiDay& operator+=(const tCentiDay& rhs) { mD+=rhs.getL(); return *this;}
        tCentiDay& operator-=(const tCentiDay& rhs) { ASSERT(rhs.getL()<=mD); mD-=rhs.getL(); return *this;}

    private:
        unsigned long mD;
};

class monthIndex;

// can represent any date (including weekend days)
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
        std::string getStr_SafeMonth() const;
        std::string getAsGoogleNewDate() const;
        std::string getStr_FileName() const;
        std::string getYYYY_MM_DD() const;

        void setForever();
        void setToStart();
        bool isForever() const;

        boost::gregorian::date getGregorian() const;
        monthIndex getMonthIndex() const;
        simpledate getEndofMonth() const;

    public:
        static bool isWeekend(const simpledate d);
        
    private:
        simpledate parseDateStringDDMMYY(std::string datestr) const;

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

        operator unsigned long() const {return mN;}

        std::string getString() const;

        unsigned long workingDaysInMonth() const;

    public:
        boost::gregorian::date getFirstMonthDay() const; 
        boost::gregorian::date getLastMonthDay() const;

    private:
        unsigned long mN;
};

// simple class for coordinate transformation from calendar date (local timezone) to number of working days from start date.
// can only hold dates that are workdays (not weekends).
class workdate : public simpledate
{
    public:
        workdate();                     // set the date as start date from settings.
        workdate(simpledate s);
        workdate(std::string datestr);  // set the date based on a dd/mm/yy string.
        workdate(const workdate &obj);  // copy ctor
        workdate(const boost::gregorian::date & d);
        workdate(unsigned long dayIndex);

        bool setclip(std::string datestr);  // set to the next workday after start from a dd/mm/yy string. 
        void decrementWorkDay();
        void incrementWorkDay();
        unsigned long getDayAsIndex() const;

        static simpledate snapWorkDay_forward(simpledate d);
        static simpledate snapWorkDay_backward(simpledate d);
        static unsigned long countWorkDays(simpledate dA, simpledate dB); // half open interval.

        static simpledate WorkDays2Date(unsigned long ndays);
        static unsigned long Date2WorkDays(simpledate d0);


    public:
        friend workdate operator+(const workdate& lhs, unsigned long rhs); // advance x work days.
};


class itemdate_test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( itemdate_test );
    CPPUNIT_TEST( itemdate_test1 );
    CPPUNIT_TEST( itemdate_test2 );
    CPPUNIT_TEST( itemdate_test3 );
    CPPUNIT_TEST( itemdate_test4 );
    CPPUNIT_TEST( itemdate_test5 );
    CPPUNIT_TEST_SUITE_END();

    public:
        void itemdate_test1();
        void itemdate_test2();
        void itemdate_test3();
        void itemdate_test4();
        void itemdate_test5();
};


// simple class to store a range of leave dates (not workdays!) - closed interval
class leaverange
{
    public:
        leaverange(std::string s); // closed interval.
        bool isEmpty() const;
        void setEmpty();

        unsigned long getStartasIndex() const;
        unsigned long getEndasIndex() const; // Half Open interval!!
        
        unsigned long holidayDaysInMonth(unsigned long month) const; // number of work days in the month that are holidays.

        std::string getString() const;

        void advance(workdate newStart);

        private:
            simpledate mStart;      // closed interval.
            simpledate mEnd;        // closed interval.
};


#endif