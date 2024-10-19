#ifndef __LATEX_WRITER_H
#define __LATEX_WRITER_H

#include <fstream>
#include "scheduler.h"

// https://www.youtube.com/watch?v=BlD6jCGVU4A
class LatexWriter
{
    public:
        LatexWriter();
        void createPDFReport(const scheduler::scheduler & s) const;

    private:
        void runLatex();

        void createTex(const scheduler::scheduler & s) const;
        void outputrow(const scheduler::scheduledproject & z, std::ofstream & ofs) const;

        void starttable(const std::string title, std::ofstream & ofs) const;
        void endtable(std::ofstream & ofs) const;

        const std::string mBaseName;
};

#endif
