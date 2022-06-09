#ifndef __PUBLIC_HOLIDAYS_H
#define __PUBLIC_HOLIDAYS_H

#include <string>
#include "workdate.h"

namespace inputfiles
{

    class publicholidays
    {
    public:
        publicholidays();

        const std::vector<leaverange> & getHolidays() const;

        void save_public_holidays_CSV(std::ostream &os) const;

        void advance(workdate newStart);

    private:
        void _load_public_holidays();

    private:
        std::vector<leaverange> mPublicHolidays;
    };

} // namespace

#endif
