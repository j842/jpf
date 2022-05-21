#include "inputfiles_publicholidays.h"
#include "simplecsv.h"
#include "globallogger.h"

namespace inputfiles
{

    publicholidays::publicholidays() : mPublicHolidaysString(load_public_holidays())
    {
    }

    std::wstring publicholidays::getStr() const
    {
        return mPublicHolidaysString;
    }

    std::wstring publicholidays::load_public_holidays() const
    {
        std::wstring phs;
        simplecsv ph("publicholidays.csv");

        if (!ph.openedOkay())
        {
            logerror("Could not read the public holidays from publicholidays.csv");
            return "";
        }

        std::vector<std::wstring> row;
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

        std::vector<std::wstring> vd;
        simplecsv::splitcsv(mPublicHolidaysString, vd);
        for (auto d : vd)
            os << simplecsv::makesafe(d) << std::endl;
    }

    void publicholidays::advance(workdate newStart)
    {
        advanceLeaveString(mPublicHolidaysString, newStart);
    }

} // namespace