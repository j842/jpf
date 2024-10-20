// auto-generated cpp file.
// created from files in includes/verbatim/input.

#include <string>
#include <fstream>

#include "generate_input.h"

/*static*/ bool generate_input::output(std::string path)
{
   if (!output_backlog_apps_csv(path)) return false;
   if (!output_backlog_devops_csv(path)) return false;
   if (!output_backlog_performance_csv(path)) return false;
   if (!output_backlog_product_csv(path)) return false;
   if (!output_backlog_web_csv(path)) return false;
   if (!output_projects_csv(path)) return false;
   if (!output_publicholidays_csv(path)) return false;
   if (!output_settings_csv(path)) return false;
   if (!output_teams_csv(path)) return false;
    return true;
}

// end of auto-generated cpp file.


// -----------------------------------------------------------------------------------------------------------
//                                   backlog-apps.csv
//

/*static*/ bool generate_input::output_backlog_apps_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("backlog-apps.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Project,ID,Description,Min Calendar WorkDays,Remaining DevDays Total,Earliest Start,Blocking,Contributing,Depends On,Tags,Comments
Android App,Ap.01,Update App to Android 17,0,10.00,,Stephen,"Blair, Carly",,,
Android App,Ap.02,Release Milestone,0,0.00,,,,,,
iPhone App,Ap.03,Port from Android,0,15.00,,Blair,,,,
Desktop Client,Ap.04,Write whole desktop client,0,22.00,,Carly,,Ap.01,,
Concrete Sculpture,Ap.05,Build a gorgeous sculpture,0,12.00,,"Stephen, Blair",Carly,,,
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------------
//                                   backlog-devops.csv
//

/*static*/ bool generate_input::output_backlog_devops_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("backlog-devops.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Project,ID,Description,Min Calendar WorkDays,Remaining DevDays Total,Earliest Start,Blocking,Contributing,Depends On,Tags,Comments
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------------
//                                   backlog-performance.csv
//

/*static*/ bool generate_input::output_backlog_performance_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("backlog-performance.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Project,ID,Description,Min Calendar WorkDays,Remaining DevDays Total,Earliest Start,Blocking,Contributing,Depends On,Tags,Comments
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------------
//                                   backlog-product.csv
//

/*static*/ bool generate_input::output_backlog_product_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("backlog-product.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Project,ID,Description,Min Calendar WorkDays,Remaining DevDays Total,Earliest Start,Blocking,Contributing,Depends On,Tags,Comments
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------------
//                                   backlog-web.csv
//

/*static*/ bool generate_input::output_backlog_web_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("backlog-web.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Project,ID,Description,Min Calendar WorkDays,Remaining DevDays Total,Earliest Start,Blocking,Contributing,Depends On,Tags,Comments
Webserver,We.01,Logins,0,10.00,,Stu,Mark,,,
Webserver,We.02,Security,0,5.00,,Sarah C,,We.01,,
Webserver,We.03,Home page,0,15.00,,Mark,,We.01,,
Webserver,We.04,Management Component,0,5.00,,Sarah C,Stu,"We.01, We.03",,Depends on both logins and home page.
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------------
//                                   projects.csv
//

/*static*/ bool generate_input::output_projects_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("projects.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Project ID,Project Name,Description,BAU or New,Comments
Webserver,Webserver,Create a new webserver,BAU,,
Android App,Android App,Android App to manage accounts,BAU,,
iPhone App,iPhone App,iPhone port of the Android app,New,,
Desktop Client,Windows Desktop Client,Desktop client for managing accounts,New,Needed Dave from Taranaki,
Concrete Sculpture,Enormous Concrete Sculpture,Support for customers who like concrete,New,,
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------------
//                                   publicholidays.csv
//

/*static*/ bool generate_input::output_publicholidays_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("publicholidays.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Public Holdiay (Date or Range)
6/6/22
24/6/22
24/10/22
26/12/22
27/12/22
2/1/23
3/1/23
6/1/23
7/4/23
25/4/23
5/6/23
14/7/23
23/10/23
25/12/23
26/12/23
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------------
//                                   settings.csv
//

/*static*/ bool generate_input::output_settings_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("settings.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Setting Name,Value,Description
inputversion,15,Input file format version
startdate,02/05/22,Start of scheduled period
enddate,30/12/22,Last month to display in monthly graphs
costperdevday,500,Cost per developer per day (including overheads)
title,Demo Project Forecast,Title for reports
port,5000,Port to use when webserver run
loglevel,DEBUG,"Logging level (Debug, Info, Warning, or Error)"
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------------
//                                   teams.csv
//

/*static*/ bool generate_input::output_teams_csv(std::string path)
{
    if (path.length()==0) return false;
    if (path[path.length()-1]!='/')
        path.push_back('/');
   std::ofstream os(path+std::string("teams.csv"));
   if (!os.is_open())
      return false;

   os << 
R"LITERAL(Team,Person,"%EFT Projects","%EFT Other BAU","%EFT Overhead for Projects (mgmt, test)","Upcoming Leave (first + last day on leave)"
Web,Stu,80,10,10,16/5/22-17/5/22
Web,Mark,90,10,0,
Web,Sarah C,90,10,0,
Apps,Stephen,80,10,10,
Apps,Blair,100,0,0,
Apps,Carly,100,0,0,
DevOps,Fred,100,0,0,
Product,Sarah K,100,0,0,
Performance,Greg,100,0,0,
)LITERAL";

   os.close();
   return true;
}
// -----------------------------------------------------------------------------------------------------------



