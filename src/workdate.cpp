#include <string>
#include <iostream>
#include <boost/date_time.hpp>

#include "workdate.h"
#include "utils.h"
#include "settings.h"

simpledate::simpledate() : mD(boost::gregorian::day_clock::local_day()) {}
simpledate::simpledate(std::string datestr) : mD(parseDateStringDDMMYY(datestr).getGregorian()) {}
simpledate::simpledate(const boost::gregorian::date &d) : mD(d) {}

std::string simpledate::getStr() const
{
    // https://www.boost.org/doc/libs/1_79_0/doc/html/date_time/date_time_io.html
    const std::locale fmt(std::locale::classic(),
                          new boost::gregorian::date_facet("%d/%m/%y"));
    return _getstr(fmt);
}

std::string simpledate::getStr_short() const
{
    const std::locale fmt(std::locale::classic(),
                          new boost::gregorian::date_facet("%d %b"));
    return _getstr(fmt);
}

std::string simpledate::getStrGantt() const
{
    const std::locale fmt(std::locale::classic(),
                          new boost::gregorian::date_facet("%e/%m/%y"));
    std::string d = _getstr(fmt);
    removewhitespace(d);
    return d;
}

std::string simpledate::getStr_nice_long() const
{
    const std::locale fmt(std::locale::classic(),
                          new boost::gregorian::date_facet("%A %B %d, %Y"));
    std::string d = _getstr(fmt);
    return d;
}

std::string simpledate::getStr_nice_short() const
{
    const std::locale fmt(std::locale::classic(),
                          new boost::gregorian::date_facet("%e %b %Y"));
    std::ostringstream oss;
    oss.imbue(fmt);
    oss << mD;
    return oss.str();
}

simpledate simpledate::parseDateStringDDMMYY(std::string datestr) const
{
    removewhitespace(datestr);

    if (datestr.length()==0)
        return gSettings().startDate().getGregorian();

    std::vector<int> v;
    int s = 0;
    for (unsigned int i = 0; i <= datestr.size(); ++i)
        if (i == datestr.size() || datestr[i] == '/')
        {
            int datecomponent = atoi(datestr.substr(s, i - s).c_str());
            ASSERT(datecomponent >= 0);
            v.push_back(datecomponent);
            s = i + 1;
        }
    if (v.size() < 3)
        TERMINATE("Malformed date string: " + datestr);
    if (v[2] < 2000)
        v[2] += 2000;

    boost::gregorian::date d1(v[2], v[1], v[0]);
    return d1;
}

// private.
std::string simpledate::_getstr(const std::locale &fmt) const
{
    if (isForever())
        return "forever";

    std::ostringstream oss;

    oss.imbue(fmt);
    oss << mD;
    return oss.str();
}

std::string simpledate::getAsGoogleNewDate() const
{
    return S() << "new Date(" << mD.year() << ", " << mD.month() - 1 << ", " << mD.day() - 1 << ")";
}

boost::gregorian::date simpledate::getGregorian() const
{
    return mD;
}

void simpledate::setForever()
{
    mD = boost::gregorian::date(boost::gregorian::pos_infin);
}
void simpledate::setToStart()
{
    mD = gSettings().startDate().getGregorian();
}
bool simpledate::isForever() const
{
    return (mD.is_infinity());
}

monthIndex simpledate::getMonthIndex() const
{
    return monthIndex(getGregorian());
}

simpledate simpledate::getEndofMonth() const
{
    return getGregorian().end_of_month();
}

bool simpledate::isWeekend(const simpledate d)
{
    return (d.getGregorian().day_of_week()== boost::date_time::Saturday || d.getGregorian().day_of_week()== boost::date_time::Sunday);
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

workdate::workdate() : simpledate(gSettings().startDate())
{
}

workdate::workdate(simpledate s) : simpledate(snapWorkDay_forward(s.getGregorian()))
{
    ASSERT(mD >= gSettings().startDate().getGregorian());    
    ASSERT(!isWeekend(mD));
}

workdate::workdate(std::string datestr) : simpledate(snapWorkDay_forward(simpledate(datestr)))
{
    ASSERT(mD >= gSettings().startDate().getGregorian());
    ASSERT(!isWeekend(mD));
}

workdate::workdate(const workdate &obj) : simpledate(obj.getGregorian())
{
    ASSERT(mD >= gSettings().startDate().getGregorian());
    ASSERT(!isWeekend(mD));
}

workdate::workdate(unsigned long dayIndex) : simpledate()
{
    mD = WorkDays2Date(dayIndex).getGregorian();
    ASSERT(mD >= gSettings().startDate().getGregorian());
    ASSERT(!isWeekend(mD));
}

workdate::workdate(const boost::gregorian::date &d) : simpledate(snapWorkDay_forward(d))
{
    ASSERT(mD >= gSettings().startDate().getGregorian());
    ASSERT(!isWeekend(mD));
}

bool workdate::setclip(std::string datestr) // set the date based on a dd/mm/yy string. If < start, det to start.
{
    if (datestr.length() == 0 || simpledate(datestr) < gSettings().startDate())
        mD = gSettings().startDate().getGregorian();
    else
        mD = snapWorkDay_forward(simpledate(datestr)).getGregorian();
    return true;
}

void workdate::decrementWorkDay()
{
    unsigned long mDay = getDayAsIndex();
    --mDay;
    mD = workdate(mDay).getGregorian();
}


void workdate::incrementWorkDay()
{
    boost::gregorian::day_iterator di(mD);
    ++di;
    while (isWeekend(*di)) ++di;
    mD=*di;
}

unsigned long workdate::getDayAsIndex() const
{
    return countWorkDays(gSettings().startDate(), *this);
}


simpledate workdate::snapWorkDay_forward(simpledate d)
{
    boost::gregorian::date d0 = d.getGregorian();
    if (d0.day_of_week() == boost::date_time::Saturday)
        return d0 + boost::gregorian::days(2);
    if (d0.day_of_week() == boost::date_time::Sunday)
        return d0 + boost::gregorian::days(1);
    return d0;
}

// half open interval.
unsigned long workdate::countWorkDays(simpledate dA, simpledate dB)
{
    boost::gregorian::date d0 = dA.getGregorian();
    boost::gregorian::date d1 = dB.getGregorian();
    long ndays = (d1 - d0).days();                       // would add +1 for inclusive

    if (ndays==0)
        return 0; 

    long fullwks = (d1-d0).days()/7;
    long numdays = 5*fullwks;

    // now adjust for ends. day_of_week is [0,..,6] for [Sunday,...,Saturday]
    long leftover = (d1-d0).days() - fullwks*7;

    // handle end bits. Fiddly.
    int startDoW = d0.day_of_week(); // [0,...,6]
    int endDoW = startDoW + leftover; // [0,...,12]

    startDoW=std::max(startDoW,1); // Advance past Sunday (day 0)
    if (startDoW==6) startDoW=8;  // Sat advanced to Mon. startDoW can't be day 7.
    else if (endDoW>7) endDoW-=2; // End is past weekend (>=Mon)... delete weekend. Note the 'else' - we only do this if we didn't skip the weekend with startDoW! 
    else if (endDoW==7) endDoW=6;
    numdays += std::max(endDoW-startDoW,0);
    return numdays;
}

unsigned long workdate::Date2WorkDays(simpledate d0)
{
    return countWorkDays(gSettings().startDate(), d0);
}

// convert back to the correct workday, mDay days from today (next workday from today is day 0).
simpledate workdate::WorkDays2Date(unsigned long ndays)
{
    boost::gregorian::date d0 = gSettings().startDate().getGregorian();

    unsigned long dayofweek = d0.day_of_week();
    unsigned long nwkends = 2 * ((ndays + dayofweek - 1) / 5); // Sat + Sun's.

    d0 += boost::gregorian::days(ndays + nwkends);

    ASSERT(d0 == snapWorkDay_forward(d0).getGregorian());
    return d0;
}


// -----------------------------------------------------------

monthIndex::monthIndex(simpledate d)
{
    boost::gregorian::date d0 = gSettings().startDate().getGregorian();
    boost::gregorian::date d1 = d.getGregorian();
    ASSERT(d1>=d0);
    mN = (d1.year() - d0.year()) * 12 + d1.month() - d0.month();
}

boost::gregorian::date monthIndex::getFirstMonthDay() const
{
    using namespace boost::gregorian;
    date d(gSettings().startDate().getGregorian());
    date d2(d.year(),d.month(),1);

    if (mN>0)
    {
        month_iterator mit(d2,mN);
        ++mit;
        d2=*mit;
    }
    return d2;
}
boost::gregorian::date monthIndex::getLastMonthDay() const
{
    boost::gregorian::date d(getFirstMonthDay());
    return d.end_of_month();
}

std::string monthIndex::getString() const
{
    const std::locale fmt(std::locale::classic(),
                          new boost::gregorian::date_facet("%b %y"));

    std::ostringstream oss;
    oss.imbue(fmt);
    oss << getFirstMonthDay();
    return oss.str();
}

workdate operator+(const workdate& lhs, unsigned long rhs) // advance x work days.
{
    unsigned long wdays = workdate::Date2WorkDays(lhs);
    wdays += rhs;
    workdate rval = workdate::WorkDays2Date(wdays).getGregorian();
    return rval;
}

unsigned long monthIndex::workingDaysInMonth() const
{
    return workdate::countWorkDays(
        getFirstMonthDay(),        
        monthIndex(mN+1).getFirstMonthDay()
    );
}


// ------------------------------------------------------------------------------


// ------------------------------------------------------------------------------

// closed interval
leaverange::leaverange(std::string s)
{
    if (s.length()==0)
        setEmpty();
    else
    {
        std::vector<std::string> strs;
        boost::split(strs, s, boost::is_any_of("-"));
        if (strs.size() == 1)
            strs.push_back(strs[0]);
        ASSERT(strs.size() == 2);
        mStart = simpledate(strs[0]);
        mEnd = simpledate(strs[1]);
    }
}

bool leaverange::isEmpty() const
{
    return (mStart.isForever() || mEnd.isForever());
}
void leaverange::setEmpty()
{
    mStart.setForever();
    mEnd.setForever();
}


unsigned long leaverange::getStartasIndex() const
{
    unsigned long ndx = workdate(mStart).getDayAsIndex(); // advance to first work day.
    return ndx;
}
unsigned long leaverange::getEndasIndex() const // Half Open interval! 
{
    if (simpledate::isWeekend(mEnd))
        return workdate(mEnd).getDayAsIndex(); // advance to first work day. 

    return workdate(mEnd).getDayAsIndex() + 1; // already on work day. map closed to open interval.
}

std::string leaverange::getString() const
{
    return S() << mStart.getStr() << "-"<< mEnd.getStr();
}

void leaverange::advance(workdate newStart)
{
    if (isEmpty())
        return;
    boost::gregorian::date ns = newStart.getGregorian();
    if (mEnd<ns)
        setEmpty();
    else
        if (mStart<ns)
            mStart=ns;
}

unsigned long leaverange::holidayDaysInMonth(unsigned long month) const // number of work days in the month that are holidays.
{
    unsigned long tally=0;
    if (monthIndex(mStart)>month || monthIndex(mEnd)<month)
        return 0;
    workdate wd(mStart);
    while (wd<mEnd)
    {
        unsigned long wdm = wd.getMonthIndex();
        if (wdm==month)
            ++tally;
        if (wdm>month)
            break;
        wd.incrementWorkDay();
    }
    return tally;
}

// ---------------


CPPUNIT_TEST_SUITE_REGISTRATION( simpledate_test );


void simpledate_test::simpledate_test0()
{
    simpledate d1("15/5/22"); //sunday
    simpledate d2("16/5/22"); //monday
    simpledate d2b("17/5/22");
    CPPUNIT_ASSERT_MESSAGE( "Workday calc wrong", workdate::snapWorkDay_forward(d1)==d2 );
    CPPUNIT_ASSERT_MESSAGE( "duration calc wrong", workdate::countWorkDays(d1,d2)==0);
    CPPUNIT_ASSERT_MESSAGE( "duration calc wrong", workdate::countWorkDays(d1,d1)==0);
    CPPUNIT_ASSERT_MESSAGE( "duration calc wrong", workdate::countWorkDays(d1,d2b)==1);
    CPPUNIT_ASSERT_MESSAGE( "duration calc wrong", workdate::countWorkDays(d2,d2b)==1);
}

void simpledate_test::simpledate_test1()
{
    simpledate d2("16/5/22"); //monday
    simpledate d3("18/5/22"); //wednesday
    CPPUNIT_ASSERT(d2.isForever()==false);
    CPPUNIT_ASSERT(d2!=d3);
    CPPUNIT_ASSERT(d2==d2);
    CPPUNIT_ASSERT(workdate::countWorkDays(d2,d3) == 2);  
    simpledate d4("21/5/22");
    simpledate d5("22/5/22");
    simpledate d6("23/5/22");
    CPPUNIT_ASSERT(workdate::countWorkDays(d2,d4)==5);
    CPPUNIT_ASSERT(workdate::countWorkDays(d2,d5)==5);
    CPPUNIT_ASSERT(workdate::countWorkDays(d2,d6)==5);
}
void simpledate_test::simpledate_test2()
{
    simpledate d2("16/5/22"); //monday
    simpledate d7("30/5/22"); //
    CPPUNIT_ASSERT(workdate::countWorkDays(d2,d7)==10);
}
void simpledate_test::simpledate_test3()
{
    simpledate d0("16/5/22");
    boost::gregorian::day_iterator dayit(d0.getGregorian());
    //if (d0.day_of_week() == boost::date_time::Sunday)
    for (unsigned int i = 0; i < 400; ++i, ++dayit)
    {
        int count = 0;
        int wkndsbtw= 0;

        simpledate d2(*dayit);
        boost::gregorian::day_iterator day2it(d2.getGregorian());
        for (unsigned int j = 0; j < 400; ++j, ++day2it)
        {
            CPPUNIT_ASSERT_MESSAGE( S()<< simpledate(*dayit).getStr() << " -> " << simpledate(*day2it).getStr() <<"  : " <<
                "Count = "<<count<<"  Wknds = "<<wkndsbtw <<"   ...  countworkdays = "<< (int)workdate::countWorkDays(*dayit,*day2it),   (int)workdate::countWorkDays(*dayit,*day2it) == count-wkndsbtw) ;

            if (day2it->day_of_week()== boost::date_time::Saturday || day2it->day_of_week() == boost::date_time::Sunday)
                wkndsbtw++;
            count++;
        }
    }
}
void simpledate_test::simpledate_test4()
{
    simpledate d0("16/5/22");
    simpledate d1("22/5/22");
    CPPUNIT_ASSERT( workdate::countWorkDays(d0,d1)==5 );
}


CPPUNIT_TEST_SUITE_REGISTRATION( itemdate_test );


void itemdate_test::itemdate_test1()
{
    gSettings().setSettings(simpledate("16/5/22"),simpledate("12/12/22"),5000);

    workdate s( gSettings().startDate() );

    CPPUNIT_ASSERT( s.getDayAsIndex() == 0);

    s.incrementWorkDay();
    CPPUNIT_ASSERT( s.getDayAsIndex() == 1);

    auto g = gSettings().startDate().getGregorian();
    boost::gregorian::day_iterator di(g,14);
    ++di;
    CPPUNIT_ASSERT( workdate(*di).getDayAsIndex()  == 10);
}
void itemdate_test::itemdate_test2()
{
    CPPUNIT_ASSERT( workdate::countWorkDays( simpledate("1/5/22"),simpledate("1/6/22")) == 22  );

    monthIndex mI(simpledate("16/5/22"));

    CPPUNIT_ASSERT( mI.getFirstMonthDay() == simpledate("1/5/22").getGregorian());
    CPPUNIT_ASSERT( monthIndex(mI+1).getFirstMonthDay() == simpledate("1/6/22").getGregorian());

    CPPUNIT_ASSERT_MESSAGE( S() << "Days in May 2022: 22, mI says " << mI.workingDaysInMonth(),   mI.workingDaysInMonth() == 22);
}
void itemdate_test::itemdate_test3()
{
    gSettings().setSettings(simpledate("16/5/22"),simpledate("12/12/22"),5000);

    srand(time(NULL));
    for (unsigned int i=0;i<1000;++i)
    {
        unsigned long date = rand()%1000;
        workdate d(date);
        boost::gregorian::date d2 = d.getGregorian();

        workdate d3(d2);
        unsigned long date2 = d3.getDayAsIndex();
        CPPUNIT_ASSERT(date2==date);
    }
}
void itemdate_test::itemdate_test4()
{

}
void itemdate_test::itemdate_test5()
{

}
