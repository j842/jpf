#ifndef __generate_input__H
#define __generate_input__H

#include <string>

class generate_input
{
    public:
        static bool output(std::string path); // recreate output files. Path needs to end in /

    private:
   static bool output_backlog_apps_csv(std::string path);
   static bool output_backlog_devops_csv(std::string path);
   static bool output_backlog_performance_csv(std::string path);
   static bool output_backlog_product_csv(std::string path);
   static bool output_backlog_web_csv(std::string path);
   static bool output_projects_csv(std::string path);
   static bool output_publicholidays_csv(std::string path);
   static bool output_settings_csv(std::string path);
   static bool output_teams_csv(std::string path);
};

#endif
