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
        void createCSVandWebsite(const scheduler::scheduler & s) const;

    private:
        void CopyHTMLFolder() const;
        void copy_site() const;

        void write_projectbacklog_csv(const scheduler::scheduler & s) const;
        void write_teams_csv(const scheduler::scheduler & s) const;
        void write_projectgantt_csv(const scheduler::scheduler & s) const;
        void write_basevars(const scheduler::scheduler & s) const;
        void write_peopleeffortbymonth_csv(const scheduler::scheduler & s) const;
        void write_people_csv(const scheduler::scheduler & s) const;
        void write_months_csv(const scheduler::scheduler & s) const;
        void write_peoplebacklog(const scheduler::scheduler & s) const;
        void write_settings(const scheduler::scheduler & s) const;
        void write_projects_csv(const scheduler::scheduler &s) const;

        void write_projecttypes(
            const scheduler::scheduler & s,
            const std::vector<std::vector<double>> & DevDaysTally,
            const std::vector<scheduler::tProjectInfo> &ProjectInfo
            ) const;
        void write_projectcostbymonth(
            const scheduler::scheduler & s,
            const std::vector<std::vector<double>> & DevDaysTally,
            const std::vector<scheduler::tProjectInfo> &ProjectInfo
            ) const;
        void write_projectcosttotal(
            const scheduler::scheduler & s,
            const std::vector<std::vector<double>> & DevDaysTally,
            const std::vector<scheduler::tProjectInfo> &ProjectInfo
            ) const;
        void write_projecttypepercents(
            const scheduler::scheduler & s,
            const std::vector<std::vector<double>> & DevDaysTally,
            const std::vector<scheduler::tProjectInfo> &ProjectInfo
            ) const;

        void recreate_Directory(std::string path) const;


        void run_jekyll() const;

        unsigned long get_maxmonth(const scheduler::scheduler & s) const;

        void write_all_tag_files(const scheduler::scheduler &s) const;
        void write_task_tag_file(const scheduler::scheduler &s, const std::string tag) const;
        void write_project_tag_file(const scheduler::scheduler &s, const std::string tag) const;
};


#endif