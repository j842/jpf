#include <string>
#include <iostream>
#include <boost/date_time.hpp>

#include "itemdate.h"
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

simpledate simpledate::parseDateStringDDMMYY(std::string datestr)
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

simpledate simpledate::nextWorkDay(simpledate d)
{
    boost::gregorian::date d0 = d.getGregorian();
    if (d0.day_of_week() == boost::date_time::Saturday)
        return d0 + boost::gregorian::days(2);
    if (d0.day_of_week() == boost::date_time::Sunday)
        return d0 + boost::gregorian::days(1);
    return d0;
}

// half open interval.
unsigned long simpledate::countWorkDays(simpledate dA, simpledate dB)
{
    boost::gregorian::date d0 = dA.getGregorian();
    boost::gregorian::date d1 = dB.getGregorian();
    long ndays = (d1 - d0).days();                       // would add +1 for inclusive
    long nwkends = 2 * ((ndays + d0.day_of_week()) / 7); // 2*Saturdays
    if (d0.day_of_week() == boost::date_time::Sunday)
        ++nwkends;
    if (d1.day_of_week() == boost::date_time::Saturday)
        --nwkends;
    return ndays - nwkends;
}

unsigned long simpledate::Date2WorkDays(simpledate d0)
{
    return countWorkDays(gSettings().startDate(), d0);
}

// convert back to the correct workday, mDay days from today (next workday from today is day 0).
simpledate simpledate::WorkDays2Date(unsigned long ndays)
{
    boost::gregorian::date d0 = gSettings().startDate().getGregorian();

    unsigned long dayofweek = d0.day_of_week();
    unsigned long nwkends = 2 * ((ndays + dayofweek - 1) / 5); // Sat + Sun's.

    d0 += boost::gregorian::days(ndays + nwkends);

    //    std::cout << getToday() << " " << ndays << " " << nwkends << " " << d0 << std::endl;

    ASSERT(d0 == nextWorkDay(d0).getGregorian());
    return d0;
}

monthIndex simpledate::getMonthIndex() const
{
    return monthIndex(getGregorian());
}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

itemdate::itemdate() : simpledate(gSettings().startDate())
{
}

itemdate::itemdate(std::string datestr) : simpledate(nextWorkDay(parseDateStringDDMMYY(datestr)))
{
    ASSERT(mD >= gSettings().startDate().getGregorian());
}

itemdate::itemdate(const itemdate &obj) : simpledate(obj.getGregorian())
{
    ASSERT(mD >= gSettings().startDate().getGregorian());
}

itemdate::itemdate(unsigned long dayIndex) : simpledate()
{
    mD = WorkDays2Date(dayIndex).getGregorian();
    ASSERT(mD >= gSettings().startDate().getGregorian());
}

itemdate::itemdate(const boost::gregorian::date &d) : simpledate(nextWorkDay(d))
{
    ASSERT(mD >= gSettings().startDate().getGregorian());
}

bool itemdate::setclip(std::string datestr) // set the date based on a dd/mm/yy string. If < start, det to start.
{
    if (datestr.length() == 0 || parseDateStringDDMMYY(datestr) < gSettings().startDate())
        mD = gSettings().startDate().getGregorian();
    else
        mD = nextWorkDay(parseDateStringDDMMYY(datestr)).getGregorian();
    return true;
}

void itemdate::decrement()
{
    unsigned long mDay = getDayAsIndex();
    --mDay;
    mD = itemdate(mDay).getGregorian();
}

void itemdate::increment()
{
    unsigned long mDay = getDayAsIndex();
    ++mDay;
    mD = itemdate(mDay).getGregorian();   
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
    if (mN==0)
        return d;

    month_iterator itr(d,mN); // use an iterator so we don't get trapped by end of month snapping and unexpected results - see Reversibility of Operations Pitfall in boost docs.
    ++itr; // jumps mN steps in one go.

    return *itr;
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

itemdate operator+(const itemdate& lhs, unsigned long rhs) // advance x work days.
{
    unsigned long wdays = simpledate::Date2WorkDays(lhs);
    wdays += rhs;
    itemdate rval = simpledate::WorkDays2Date(wdays).getGregorian();
    return rval;
}


// ------------------------------------------------------------------------------

unsigned long itemdate::getDayAsIndex() const
{
    return countWorkDays(gSettings().startDate(), *this);
}

daterange::daterange(std::string s, tIntervalTypes t)
{ // parse date range string. Could be date, or date-date (inclusive).

    std::vector<std::string> strs;
    boost::split(strs, s, boost::is_any_of("-"));
    if (strs.size() == 1)
    {
        strs.push_back(strs[0]);
        t = kClosedInterval; // treat as closed.
    }
    ASSERT(strs.size() == 2);
    mStart.setclip(strs[0]);
    mEnd.setclip(strs[1]);

    if (t == kClosedInterval) // convert to half open.
        mEnd.increment();
}
itemdate daterange::getStart() const
{
    return mStart;
}

itemdate daterange::getEnd() const
{
    return mEnd;
}

void daterange::setStart(itemdate start)
{
    mStart = start;
}
void daterange::setEnd(itemdate end, tIntervalTypes t)
{
    mEnd = end;
    if (t == kClosedInterval)
        mEnd.increment();

    ASSERT((mEnd.getGregorian()-mStart.getGregorian()).days()>0); // non-empty interval.
}

std::string daterange::getRangeAsString() const
{
    boost::gregorian::date_duration dd = mEnd.getGregorian()-mStart.getGregorian();
    if (dd.days() <= 1)
        return mStart.getStr(); // empty set.

    std::ostringstream oss;
    oss << mStart.getStr() << "-" << mEnd.getStr();
    return oss.str();
}
