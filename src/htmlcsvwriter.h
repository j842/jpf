#ifndef __HTMLCSVWRITER_H
#define __HTMLCSVWRITER_H

#include <fstream>
#include "scheduler.h"

class simpleDataCSV
{
    public:
        simpleDataCSV(std::string name);
        ~simpleDataCSV();

        void addrow(const std::vector<std::string> & row);

    private:
        std::ofstream mStream;
};

class HTMLCSVWriter
{
    public:
        HTMLCSVWriter();
        void createHTMLFolder(const scheduler::scheduler & s) const;

    private:
        void CopyHTMLFolder() const;
        void write_projectbacklog_csv(const scheduler::scheduler & s) const;
        void write_projectgantt_csv(const scheduler::scheduler & s) const;
        void write_basevars(const scheduler::scheduler & s) const;
        void write_peopleeffortbymonth_months_people_csvs(const scheduler::scheduler & s) const;
        void write_dashboard(const scheduler::scheduler & s) const;

        void recreate_Directory(std::string path) const;

        void copy_site() const;

        void run_jekyll() const;

        unsigned long get_maxmonth(const scheduler::scheduler & s) const;

        void write_all_tag_files(const scheduler::scheduler &s) const;
        void write_tag_file(const scheduler::scheduler &s, const std::string tag) const;

};


#endif