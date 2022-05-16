#ifndef __SCHEDULER_WORKLOG__H
#define __SCHEDULER_WORKLOG__H

#include "itemdate.h"

namespace scheduler
{

    typedef unsigned long tNdx;

    class worklogitem
    {
        public:
            worklogitem() {}
            worklogitem( simpledate d, tNdx i,tNdx p, tCentiDay ce, tCentiDay isf, tCentiDay pdsf ) :
                day(d),
                itemNdx(i),
                personNdx(p),
                chunkEffort(ce),
                itemSoFar(isf),
                personDaySoFar(pdsf)
                {
                }

        public:
            simpledate day;
            tNdx itemNdx; // index in mItems
            tNdx personNdx;
            tCentiDay chunkEffort;
            tCentiDay itemSoFar;
            tCentiDay personDaySoFar;
    };


}

#endif