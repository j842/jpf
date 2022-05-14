#include <iostream>
#include <errno.h>
#include <vector>
#include <string>
#include <filesystem>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "projects.h"
#include "teams.h"
#include "backlog.h"
#include "utils.h"
#include "settings.h"
#include "main.h"
#include "simplecsv.h"

void cMain::replace_all_input_CSV_files(projects &p, teams &t, backlog &b)
{
    for (unsigned int i = 0; i < t.size(); ++i)
    { // output team-X
        std::ofstream ofs(simplecsv::filename2path("team-" + makelower(t[i].mId) + ".csv"));
        b.save_team_CSV(ofs, i);
    }

    {
        std::ofstream ofs(simplecsv::filename2path("settings.csv"));
        gSettings().save_settings_CSV(ofs);
    }

    {
        std::ofstream ofs(simplecsv::filename2path("publicholidays.csv"));
        b.save_public_holidays_CSV(ofs);
    }

    {
        std::ofstream ofs(simplecsv::filename2path("teams.csv"));
        t.save_teams_CSV(ofs);
    }

    {
        std::ofstream ofs(simplecsv::filename2path("projects.csv"));
        p.save_projects_CSV(ofs);
    }
}

int cMain::run_refresh()
{
    try
    {
        projects p;
        teams t;
        backlog b(p, t);
        b.refresh();

        replace_all_input_CSV_files(p, t, b);
    }
    catch (TerminateRunException &pEx)
    {
        std::cerr << pEx.what() << std::endl;
        return 1;
    }
    return 0;
}

int cMain::run_console()
{
    timer tmr;

    try
    {
        projects p;
        teams t;
        backlog b(p, t);
        b.schedule();
        std::cout << std::endl
                  << std::endl;
        b.displayprojects(std::cout);
        b.createAllOutputFiles();
    }
    catch (TerminateRunException &pEx)
    {
        std::cerr << pEx.what() << std::endl;
        return 1;
    }

    std::cout << std::endl
              << "Completed in " << std::setprecision(3) << tmr.stop() << "ms." << std::endl;
    return 0;
}

int cMain::run_watch()
{
    webserver wserver(gSettings().getPort());
    watcher w(getInputPath());

    while (true)
    {
        try
        {
            projects p;
            teams t;
            backlog b(p, t);
            b.schedule();

            timer tmr;
            b.createAllOutputFiles();
            std::cout << "File output done in " << std::setprecision(3) << tmr.stop() << "ms." << std::endl;
        }
        catch (TerminateRunException &pEx)
        {
            std::vector<std::string> htmlfiles = {"index.html", "people.html", "costdashboard.html", "highlevelgantt.html", "detailedgantt.html", "rawbacklog.html"};
            for (auto &x : htmlfiles)
                backlog::outputHTMLError(getOutputPath_Html() + x, pEx.what());
            std::cerr << pEx.what() << std::endl;
        }

        std::cout << std::endl
                  << "Watching for changes. Ctrl-c to exit." << std::endl;
        w.waitforchange();
        std::cout << "Files updated...recalculating!" << std::endl;
    }
    return 0;
}

int cMain::showhelp()
{
    std::cout << std::endl;
    std::cout << "jpf " << gSettings().getJPFVersionStr() << "-" << gSettings().getJPFReleaseStr() << " is a simple auto-balancing forecasting tool for projects across multiple teams.";

    std::cout << R"(

The directory used needs to contain an input folder, with appropriate csv files in it.

usage: jpf [ mode ] DIRECTORY

Mode:
    -w, -watch      Watch the folder for changes, and update all output files as needed.
                    Also starts a webserver, displaying the HTML output files.
                    The corresponding webpages will auto-update as inputs change.
                    The port can be configured in settings.csv.

    -c, -create     Create a skeleton working tree in DIRECTORY, which includes input and 
                    output directories, with example input files. 

    -t, -test       Run unit tests.

    -r, -refresh    Refresh the input files (read, tidy, write).

    -a=dd/mm/yy     Advance start to date, decrementing work expected to be completed and
                    removing no longer relevant dates.
                
)";
    return 0;
}

// int getport(std::string s)
// {
//     int port = 5000;
//     for (unsigned int i = 0; i < s.length(); ++i)
//         if (s[i] == '=' && i < s.length() - 1)
//         {
//             port = atoi(s.c_str() + i + 1);
//         }

//     if (port <= 1024)
//         TERMINATE("Need user settable port - i.e. > 1024.");
//     return port;
// }

int cMain::create_directories()
{
    std::string pr = gSettings().getRoot();
    std::string pi = pr + "/input/";
    std::string ex = "/opt/jpf/input/";

    if (std::filesystem::exists(pi))
        TERMINATE("Can't create directories - input dir already exists: " + pi);

    checkcreatedirectory(pr);

    if (!std::filesystem::exists(ex))
        TERMINATE("Expected example files were not installed in /opt/jpf/input.");

    copy("/opt/jpf/input/", pr, std::filesystem::copy_options::recursive);
    if (!std::filesystem::exists(pi))
        TERMINATE("Input directory was not successfully created: " + pi);

    return 0;
}

bool cMain::runtests()
{
    CPPUNIT_NS::Test *suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);
    bool wasSucessful = runner.run();

    std::cout << (wasSucessful ? "Success!!" : "Fail!!") << std::endl;

    return wasSucessful;
}

int cMain::advance(std::string s)
{ // s of format -a=dd/mm/yy
    s.erase(s.begin(), s.begin() + 3);
    itemdate newStart(s);
    std::cout << " Advancing start date " << (newStart - gSettings().startDate()).getAsDurationUInt() << " workdays --> " << newStart.getStr_nice() << std::endl;

    try
    {
        projects p;
        teams t;
        backlog b(p, t);
        b.schedule();

        gSettings().advance(newStart);
        p.advance(newStart);
        t.advance(newStart);

    }
    catch (TerminateRunException &pEx)
    {
        std::cerr << pEx.what() << std::endl;
        return 1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    cMain m(argc, argv);
    return m.getrVal();
}

int cMain::getrVal() const
{
    return mrVal;
}

cMain::cMain(int argc, char **argv) : mrVal(0)
{
    mrVal = go(argc, argv);
}

int cMain::go(int argc, char **argv)
{
    if (argc <= 1 || argc > 3)
        return showhelp();

    try
    {
        // handle options which do not require a directory.
        if (argc>=2 && strlen(argv[1])>1 && argv[1][0]=='-' && tolower(argv[1][1])=='t')
            return runtests() ? 0 : 1;        

        // set directory.
        std::string directory = argv[argc - 1];
        gSettings().setRoot(directory);

        gSettings().load_settings();
        if (!gSettings().RootExists())
            TERMINATE("Root directory " + gSettings().getRoot() + " does not exist.");

        std::cout << std::endl;
        std::cout << "John's Project Forecaster " << gSettings().getJPFVersionStr() << "-" << gSettings().getJPFReleaseStr() << " - An auto-balancing forecasting tool." << std::endl
                  << std::endl;
        std::cout << "Root directory: " << gSettings().getRoot() << std::endl;

        if (argc == 2)
            return run_console();

        std::string s = argv[1];
        if (s.length() < 2)
            TERMINATE("Bad parameter: " + s);
        if (s[0] != '-')
            TERMINATE("Options must start with - : " + s);

        switch (s[1])
        {
        case 'w':
        {
            catch_ctrl_c();
            run_watch();
            return 0;
        }
        case 'a':
            return advance(s);
        case 'c':
            return create_directories();
        case 'r':
            return run_refresh();
        default:
            TERMINATE("Bad parameter: " + s);
        }
    }

    catch (const ctrlcException &e)
    {
        std::cerr << "\n\n"
                  << e.what() << '\n';
        return 1;
    }
    catch (const TerminateRunException &e)
    {
        std::cerr << "\n\n"
                  << e.what() << '\n';
        return 1;
    }

    return 0;
}
