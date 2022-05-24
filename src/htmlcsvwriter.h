#ifndef __HTMLCSVWRITER_H
#define __HTMLCSVWRITER_H

#include "scheduler.h"

class HTMLCSVWriter
{
    public:
        HTMLCSVWriter();
        void createHTMLFolder(const scheduler::scheduler & s) const;

    private:
        void CopyHTMLFolder() const;
        void write_projectbacklog_csv(const scheduler::scheduler & s) const;
};


#endif