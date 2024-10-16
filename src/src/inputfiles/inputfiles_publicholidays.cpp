#include "inputfiles_publicholidays.h"
#include "simplecsv.h"
#include "globallogger.h"

namespace inputfiles
{

    publicholidays::publicholidays()
    {
        _load_public_holidays();
    }

    // std::string publicholidays::getStr() const
    // {
    //     return mPublicHolidaysString;
    // }

    void publicholidays::_load_public_holidays()
    {
        std::string phs;
        simplecsv ph("publicholidays.csv");

        if (!ph.openedOkay())
            logerror("Could not read the public holidays from publicholidays.csv");
        else
        {
            std::vector<std::string> row;
            while (ph.getline(row, 1))
            {
                leaverange dr(row[0]);
                if (!dr.isEmpty())
                    mPublicHolidays.push_back(dr);
            }
        }
    }

    void publicholidays::save_public_holidays_CSV(std::ostream &os) const
    {
        os << "Public Holdiay (Date or Range)" << std::endl;

        for (auto & i : mPublicHolidays)
            os << simplecsv::makesafe(i.getString()) << std::endl;
    }

    void publicholidays::advance(workdate newStart)
    {
        for (auto & i : mPublicHolidays)
            i.advance(newStart);

        mPublicHolidays.erase(std::remove_if(
                                  mPublicHolidays.begin(), mPublicHolidays.end(),
                                  [](const leaverange &x)
                                  {
                                      return x.isEmpty(); // remove if empty.
                                  }),
                              mPublicHolidays.end());
    }

    const std::vector<leaverange> & inputfiles::publicholidays::getHolidays() const 
    {
        return mPublicHolidays;
    }


} // namespace

