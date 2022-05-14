#include "publicholidays.h"
#include "simplecsv.h"

publicholidays::publicholidays() : mPublicHolidaysString( load_public_holidays() )
{
}

std::string publicholidays::getStr() const
{
    return mPublicHolidaysString;
}

std::string publicholidays::load_public_holidays() const
{
    std::string phs;
    simplecsv ph("publicholidays.csv");

    if (!ph.openedOkay())
    {
        std::cerr << "Could not read the public holidays from publicholidays.csv" << std::endl;
        return "";
    }

    std::vector<std::string> row;
    while (ph.getline(row, 1))
    {
        if (phs.length() > 0)
            phs += ",";
        phs += row[0];
    }
    return phs;
}

void publicholidays::save_public_holidays_CSV(std::ostream &os) const
{
    os << "Public Holdiay (Date or Range)" << std::endl;

    std::vector<std::string> vd;
    simplecsv::splitcsv(mPublicHolidaysString, vd);
    for (auto d : vd)
        os << simplecsv::makesafe(d) << std::endl;
}

void publicholidays::advance(itemdate newStart)
{
    advanceLeaveString(mPublicHolidaysString,newStart);
}

