#ifndef __SCHEDULER_WORKLOG__H
#define __SCHEDULER_WORKLOG__H

#include "workdate.h"

namespace scheduler
{

    typedef unsigned long tNdx;

    class worklogitem
    {
        public:
            worklogitem() {}
            worklogitem( simpledate d, tNdx i,tNdx p, tCentiDay ce, tCentiDay irem, tCentiDay pdrem ) :
                day(d),
                itemNdx(i),
                personNdx(p),
                chunkEffort(ce),
                itemRemaining(irem),
                personDayRemaining(pdrem)
                {
                }

        public:
            simpledate day;
            tNdx itemNdx; // index in mItems
            tNdx personNdx;
            tCentiDay chunkEffort;
            tCentiDay itemRemaining;
            tCentiDay personDayRemaining;
    };


}

#endif