#ifndef __PUBLIC_HOLIDAYS_H
#define __PUBLIC_HOLIDAYS_H

#include <string>
#include "itemdate.h"

namespace inputfiles
{

    class publicholidays
    {
    public:
        publicholidays();

        std::string getStr() const;

        void save_public_holidays_CSV(std::ostream &os) const;

        void advance(itemdate newStart);

    private:
        std::string load_public_holidays() const;

        std::string mPublicHolidaysString;
    };

} // namespace

#endif
