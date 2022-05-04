#include <string>
#include <iostream>
#include <boost/date_time.hpp>

#include "itemdate.h"
#include "utils.h"
#include "settings.h"

itemdate::itemdate() : mDay(0)
{
}

itemdate::itemdate(std::string datestr) : mDay(0)
{
    set(datestr);
}

itemdate::itemdate(const itemdate &obj) : mDay(obj.mDay)  // copy ctor
{
}

itemdate::itemdate(unsigned int dayIndex) : mDay(dayIndex)
{
}

itemdate::itemdate(const boost::gregorian::date & d)
{
    set(d);
}

bool itemdate::setclip(std::string datestr)  // set the date based on a dd/mm/yy string. If < start, det to start.
{
     if (datestr.length()==0 || parseDateStringDDMMYY(datestr)<Today())
     {
        mDay=0;
        return true;
     }
    else
        return set(datestr);
}

bool itemdate::set(const boost::gregorian::date & d)
{   
    boost::gregorian::date d0 = Today();
    boost::gregorian::date d1 = nextWorkday(d);

    if (d1<d0) TERMINATE(S() << "Date in the past! Need to update input files: "<<d1);

    mDay=countWeekDays(d0,d1);
    ASSERT(mDay>=0);

#ifdef __DEBUG
    std::cout << d0 << " to " << d1 << " - " << mDay << " (" << (d1-d0).days() <<")"<< std::endl;
    std::cout << getStr() << std::endl;
#endif
    
    return true;
}

bool itemdate::set(std::string datestr)
{
    if (datestr.length()==0)
    {
        mDay=0;
        return true;
    }

    return set(parseDateStringDDMMYY(datestr));
}

void itemdate::decrement()
{
    ASSERT(mDay>0);
    --mDay;
}



boost::gregorian::date itemdate::Today() 
{
    return gSettings().startDate();
}

unsigned int itemdate::getEndMonth() 
{
    return getMonthFromStart(gSettings().endDate());
}


// long countWeekDays( std::string d0str, std::string d1str ) {
//     boost::gregorian::date d0(boost::gregorian::from_simple_string(d0str));
//     boost::gregorian::date d1(boost::gregorian::from_simple_string(d1str));
//     long ndays = (d1-d0).days() + 1; // +1 for inclusive
//     long nwkends = 2*( (ndays+d0.day_of_week())/7 ); // 2*Saturdays
//     if( d0.day_of_week() == boost::date_time::Sunday ) ++nwkends;
//     if( d1.day_of_week() == boost::date_time::Saturday ) --nwkends;
//     return ndays - nwkends;
// }

boost::gregorian::date itemdate::nextWorkday(boost::gregorian::date d0)
{
    if ( d0.day_of_week() == boost::date_time::Saturday ) return d0+boost::gregorian::days(2);
    if ( d0.day_of_week() == boost::date_time::Sunday ) return d0+boost::gregorian::days(1);
    return d0;
}

// half open interval.
long itemdate::countWeekDays(boost::gregorian::date d0, boost::gregorian::date d1) const
{
    long ndays = (d1-d0).days(); // would add +1 for inclusive
    long nwkends = 2*( (ndays+d0.day_of_week())/7 ); // 2*Saturdays
    if( d0.day_of_week() == boost::date_time::Sunday ) ++nwkends;
    if( d1.day_of_week() == boost::date_time::Saturday ) --nwkends;
    return ndays - nwkends;
}

boost::gregorian::date itemdate::parseDateStringDDMMYY(std::string datestr)
{
    removewhitespace(datestr);
    std::vector<int> v;
    int s=0;
    for (unsigned int i=0;i<=datestr.size();++i)
        if (i==datestr.size() || datestr[i]=='/')
            {
                int datecomponent = atoi(datestr.substr(s,i-s).c_str());
                ASSERT(datecomponent >= 0);
                v.push_back(datecomponent);
                s=i+1;
            }
    if (v.size()<3)
        TERMINATE( "Malformed date string: " + datestr);
    if (v[2]<2000) v[2]+=2000;

    boost::gregorian::date d1(v[2],v[1],v[0]);
    return d1;
}


// convert back to the correct workday, mDay days from today (next workday from today is day 0).
boost::gregorian::date itemdate::day2date(unsigned int day) const
{
    ASSERT(day>=0);
    boost::gregorian::date d0 = Today();

    long dayofweek = d0.day_of_week();

    long ndays   = day;
    long nwkends = 2*( (ndays+dayofweek-1)/5 ); // Sat + Sun's.

    d0 += boost::gregorian::days( ndays + nwkends );

//    std::cout << getToday() << " " << ndays << " " << nwkends << " " << d0 << std::endl;

    ASSERT(d0 == nextWorkday(d0));
    return d0;
}

void itemdate::setForever()
{
    mDay=UINT_MAX;
}
void itemdate::setToStart()
{
    mDay=0;
}
bool itemdate::isForever() const
{
    return (mDay==UINT_MAX);
}


// private.
std::string itemdate::getstr(const std::locale & fmt) const
{ 
    if (isForever())
        return "forever";

    std::ostringstream oss;
    oss.imbue(fmt);

    ASSERT(mDay>=0);
    boost::gregorian::date d0 = day2date(mDay);
    oss << d0;

    return oss.str();    
}


std::string itemdate::getStr() const
{   
// https://www.boost.org/doc/libs/1_79_0/doc/html/date_time/date_time_io.html
     const std::locale fmt(std::locale::classic(),
                      new boost::gregorian::date_facet("%d/%m/%y"));
    return getstr(fmt);
}

std::string itemdate::getStr_short() const
{
    const std::locale fmt(std::locale::classic(),
                      new boost::gregorian::date_facet("%d %b"));
    return getstr(fmt);
}

std::string itemdate::getStrGantt() const
{
    const std::locale fmt(std::locale::classic(),
                      new boost::gregorian::date_facet("%e/%m/%y"));
    std::string d = getstr(fmt);   
    removewhitespace(d); 
    return d;
}
        

std::string itemdate::date2strNice(boost::gregorian::date d0)
{
     const std::locale fmt(std::locale::classic(),
                      new boost::gregorian::date_facet("%e %b %Y"));
    std::ostringstream oss;
    oss.imbue(fmt);
    oss << d0;
    return oss.str();    
}

unsigned int itemdate::getMonthFromStart(boost::gregorian::date d1)
{
    boost::gregorian::date d0 = gSettings().startDate();

    unsigned int months = (d1.year() - d0.year())*12 + d1.month() - d0.month();
    return months;
}

unsigned int itemdate::getMonthFromStart() const
{
    return getMonthFromStart(day2date(mDay));
}

std::string itemdate::getMonthAsString() const
{
    const std::locale fmt(std::locale::classic(),
                      new boost::gregorian::date_facet("%b %y"));
    std::string d = getstr(fmt);   
    return d;    
}

std::string itemdate::getMonthAsString(unsigned int monthFromStart)
{
    boost::gregorian::date d0(firstdayofmonth(monthFromStart));
    itemdate id(d0);
    return id.getMonthAsString();
}


boost::gregorian::date itemdate::firstdayofmonth(unsigned int monthFromStart)
{
    using namespace boost::gregorian;
    greg_year y = Today().year();
    greg_month m = Today().month();
    date d0(y,m,1);
    d0 += boost::gregorian::months(monthFromStart);
    return d0;
}

boost::gregorian::date itemdate::lastdayofmonth(unsigned int monthFromStart)
{
    return firstdayofmonth(monthFromStart).end_of_month();
}

std::string itemdate::getAsGoogleNewDate() const
{
    boost::gregorian::date d0 = day2date(mDay);
    return S()<<"new Date("<<d0.year()<<", "<<d0.month()-1<<", "<<d0.day()-1<<")";
}

std::string itemdate::getAsDurationString() const
{
    return S() << mDay;
}

double itemdate::getAsDurationDouble() const
{
    return (double)mDay;
}

unsigned int itemdate::getDayAsIndex() const
{
    return mDay;
}
