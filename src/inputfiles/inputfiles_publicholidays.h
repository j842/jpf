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

        std::wstring getStr() const;

        void save_public_holidays_CSV(std::wostream &os) const;

        void advance(workdate newStart);

    private:
        std::wstring load_public_holidays() const;

        std::wstring mPublicHolidaysString;
    };

} // namespace

#endif
