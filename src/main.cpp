#include <iostream>
#include <errno.h>
#include <vector>
#include <string>
#include <filesystem>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "inputfiles_projects.h"
#include "inputfiles_teams.h"
#include "inputfiles_publicholidays.h"
#include "inputfiles_teambacklogs.h"
#include "inputfiles_inputset.h"
#include "generate_input.h"

#include "scheduler.h"
#include "utils.h"
#include "settings.h"
#include "main.h"
#include "simplecsv.h"
#include "colours.h"
#include "htmlcsvwriter.h"

// --------------------------------------------------------------------


void cMain::replace_all_input_CSV_files(inputfiles::inputset iset)
{
    for (unsigned int i = 0; i < iset.mB.mTeamItems.size(); ++i)
    { // output backlog-X
        std::ofstream ofs(simplecsv::filename2path(S()<<"backlog-" << makelower(iset.mT[i].mId) << ".csv"));
        iset.mB.save_team_CSV(ofs, i);
        ofs.close();
    }

    {
        std::ofstream ofs(simplecsv::filename2path("publicholidays.csv"));
        iset.mH.save_public_holidays_CSV(ofs);
        ofs.close();
    }

    {
        std::ofstream ofs(simplecsv::filename2path("teams.csv"));
        iset.mT.save_teams_CSV(ofs);
        ofs.close();
    }

    {
        std::ofstream ofs(simplecsv::filename2path("projects.csv"));
        iset.mP.save_projects_CSV(ofs);
        ofs.close();
    }
}

void cMain::replace_settings_CSV()
{
    std::ofstream ofs(simplecsv::filename2path("settings.csv"));
    gSettings().save_settings_CSV(ofs);
    ofs.close();
}

int cMain::run_refresh()
{
    try
    {
        { // load and refresh
            loginfo("Loading files...");
            inputfiles::projects p;
            inputfiles::teams t;
            inputfiles::publicholidays h;
            inputfiles::teambacklogs b(t,p);
            inputfiles::inputset iset(p,t,h,b);
            scheduler::scheduler s(iset);

            loginfo("Recreating Ids as needed...");
            s.refresh(iset);

            loginfo("Saving updated input files...");
            replace_all_input_CSV_files(iset);
            replace_settings_CSV();
        }


        { // reload from disk and schedule.
            loginfo("Rescheduling...");
            inputfiles::projects p;
            inputfiles::teams t;
            inputfiles::publicholidays h;
            inputfiles::teambacklogs b(t,p);
            inputfiles::inputset iset(p,t,h,b);
            scheduler::scheduler s(iset);

            s.schedule();
            s.createAllOutputFiles();

            HTMLCSVWriter hcw;
            hcw.createHTMLFolder(s);

            s.displayprojects_Console();
        }
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
        inputfiles::projects p;
        inputfiles::teams t;
        inputfiles::publicholidays h;
        inputfiles::teambacklogs b(t,p);
        inputfiles::inputset iset(p,t,h,b);

        scheduler::scheduler s(iset);
        s.schedule();

        s.createAllOutputFiles();

        HTMLCSVWriter hcw;
        hcw.createHTMLFolder(s);

        s.displayprojects_Console();
    }
    catch (TerminateRunException &pEx)
    {
        std::cerr << pEx.what() << std::endl;
        return 1;
    }

    loginfo(S() <<"Completed in " << std::fixed << std::setprecision(3) << tmr.stop()/1000.0 << "s.");
    return 0;
}

int cMain::run_watch()
{
    webserver wserver(gSettings().getPort());
    watcher w({getInputPath(),getInputPath_Jekyll()});

    while (true)
    {
        try
        {
            inputfiles::projects p;
            inputfiles::teams t;
            inputfiles::publicholidays h;
            inputfiles::teambacklogs b(t,p);
            inputfiles::inputset iset(p,t,h,b);

            scheduler::scheduler s(iset);
            s.schedule();

            timer tmr;
            s.createAllOutputFiles();

            HTMLCSVWriter hcw;
            hcw.createHTMLFolder(s);

            logdebug(S() << "File output done in " << std::setprecision(3) << tmr.stop() << "ms.");
        }
        catch (TerminateRunException &pEx)
        {            
            for (auto x : {
                "dashboard.html",
                "gantt_projects.html",
                "index.html",
                "people_backlog.html",
                "people_effort.html",
                "project_backlog.html",
                "tags.html"
                }
                )
                scheduler::scheduler::outputHTMLError(S()<<getOutputPath_Html() << x, pEx.what());
            logerror(pEx.what());
        }

        loginfo("Watching for changes. Ctrl-c to exit.");
        w.waitforchange();
        loginfo("Files updated...recalculating!");
    }
    return 0;
}


int cMain::run_create_directories()
{
    std::string pr = gSettings().getRoot();
    std::string pi = getInputPath();

    checkcreatedirectory(pr);

    if (!std::filesystem::exists(pi))
    {
        generate_input gi;
        checkcreatedirectory(pi);
        gi.output(pi);
        if (!std::filesystem::exists(pi))
            fatal("Input directory was not successfully created: " + pi);
        loginfo("Created "+pi);
    }
    else
        loginfo(pi+" already exists.");

    return 0;
}

bool cMain::runtests()
{
    gSettings().setMinLogLevel(kLDEBUG);
    loginfo("Running unit tests...");

    timer t;
    CPPUNIT_NS::Test *suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);
    bool wasSucessful = runner.run();

    if (wasSucessful)
        loginfo("All tests passed!");
    else
        logerror("FAIL!");

    loginfo(S()<<"Time taken: " << t.stop()/1000.0 << " s.");
    return wasSucessful;
}

int cMain::run_advance(std::string s)
{ // s of format -a=dd/mm/yy
    s.erase(s.begin(), s.begin() + 3);
    workdate newStart(s);
    loginfo(S()<<" Advancing start date " << workdate::countWorkDays(gSettings().startDate(),newStart) << " workdays --> " << newStart.getStr_nice_short());

    try
    {
        {
            inputfiles::projects p;
            inputfiles::teams t;
            inputfiles::publicholidays h;
            inputfiles::teambacklogs b(t,p);
            inputfiles::inputset iset(p, t, h, b);
            // advance and throw away scheduler.
            scheduler::scheduler s(iset);
            s.schedule();
            s.advance(newStart, iset); // advances the iset members.
            gSettings().advance(newStart);

            ASSERT(gSettings().startDate() == newStart);

            replace_all_input_CSV_files(iset);
            replace_settings_CSV();

            loginfo("Replaced input and output files.");
        }
    }
    catch (TerminateRunException &pEx)
    {
        std::cerr << pEx.what() << std::endl;
        return 1;
    }

    // new re-run from scratch.
    run_console();

    return 0;
}

// --------------------------------------------------------------------

int cMain::showhelp()
{
    std::ostream & oss = std::cout;
    oss << std::endl;
    oss << "  jpf " << gSettings().getJPFFullVersionStr() << " is a simple auto-balancing forecasting tool for projects across multiple teams.";

    oss << R"(

  The directory used needs to contain an input folder, with appropriate csv files in it.

  usage: jpf [ mode ] DIRECTORY

  Mode:
      -w, -watch      Watch the folder for changes, and update all output files as needed.
                      Also starts a webserver, displaying the HTML output files.
                      The corresponding webpages will auto-update as inputs change.
                      The port can be configured in settings.csv.

      -c, -create     Create a skeleton working tree in the current directory, 
                      with example input files. 

      -r, -refresh    Refresh the input files (read, tidy, write).

      -a=dd/mm/yy     Advance start to date, decrementing work expected to be completed and
                      removing no longer relevant dates.

      -t, -test       Run unit tests. Ignores DIRECTORY.

)";
    return 0;
}


// --------------------------------------------------------------------


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

int cMain::go(int ac, char **av)
{
    cLocalSettings localsettings;

    std::vector<std::string> argv(ac);
    for (int z=0;z<ac;++z)
        argv[z] = std::string(av[z]);
    if (argv.size()>0)
        argv.erase(argv.begin()); // drop exe name.

    std::vector<char> opts;

    for (auto & i : argv)
        if (i.length()>1 && i[0]=='-')
            opts.push_back(i[1]);

    if (opts.size()>1)
        showhelp();

    // no directory needed.
    if (opts.size()>0 && opts[0]=='t')
        return runtests() ? 0 : 1;     

    // all other options need a root directory to be specified.
    
    std::string dir;
    if (argv.size()>0 && argv.back().length() > 0 && argv.back()[0] != '-')
    {
        dir = argv.back();
        if (dir.length() > 0 && std::filesystem::exists(dir))
            dir = std::filesystem::canonical(dir);
    }
    else
    {
        if (localsettings.isLoaded() && localsettings.getSetting("input").length() > 0)
            dir = localsettings.getSetting("input");
    }

    if (dir.length()==0)
        showhelp();

    try
    {
        gSettings().setRoot(dir);
        localsettings.setSetting("input",gSettings().getRoot());

        // -c option needs root directory, but not loading settings!
        if (opts.size()>0 && opts[0]=='c')
            return run_create_directories();

        if (!gSettings().RootExists())
            TERMINATE("Root directory " + gSettings().getRoot() + " does not exist.");

        if (!std::filesystem::exists( getInputPath()+"settings.csv" ))
            fatal("The settings.csv file does not exist:\n  "+getInputPath()+"settings.csv\n\nRun with -c flag to create input directories.");

        gSettings().load_settings();

        // ----------------------------------------------------------------------------------------------
        // Settings loaded. Let's go!


        logdebug(S()<<"\n"+std::string(79,'-')<<"\n");
        std::string aaa;
        for (auto & a : argv)
            aaa+=S()<<a<<" ";
        logdebug(aaa);

        loginfo(S()<<"John's Project Forecaster " << gSettings().getJPFVersionStr() << "-" << gSettings().getJPFReleaseStr() << " - An auto-balancing forecasting tool.");
        logdebug(S()<<"Root directory: " << gSettings().getRoot());
        logdebug(S() << "Input file version is "<<gSettings().getInputVersion());


        if (opts.size()==0)
            return run_console();

        switch (opts[0])
        {
        case 'w':
        {
            catch_ctrl_c();
            run_watch();
            return 0;
        }
        case 'a':
            for (auto & i : argv)
                if (i.length()>1 && i[0]=='-' && tolower(i[1])=='a')
                    return run_advance(i);
        case 'r':
            return run_refresh();

        default:
            TERMINATE("Bad option: -" + opts[0]);
        }
    }

    catch (const ctrlcException &e)
    {
        loginfo(e.what());
        return 1;
    }
    catch (const TerminateRunException &e)
    {
        logerror(e.what());
        return 1;
    }

    return 0;
}

cLocalSettings::cLocalSettings() :
    mFilePath(S()<<getHomeDir() << ".jpf_settings"),
    mLoaded(false)
{
    if (std::filesystem::exists(mFilePath))
    {
        simplecsv csv(mFilePath,2);
        if (!csv.openedOkay())
            TERMINATE("Could not open "+mFilePath);
        std::vector<std::string> line;
        while (csv.getline(line,2))
            mSettings[line[0]]=line[1];
        mLoaded=true;
    }
}
bool cLocalSettings::isLoaded() const
{
    return mLoaded;
}
void cLocalSettings::setSetting(std::string key, std::string value)
{
    mSettings[key]=value;
    save();
}
std::string cLocalSettings::getSetting(std::string key) const
{
    for (auto & s : mSettings)
        if (iSame(s.first,key))
            return s.second;
    return "";
}
void cLocalSettings::save() const
{
    std::ofstream ofs(mFilePath);
    if (!ofs.is_open())
        TERMINATE("Could not open settings file for writing: "+mFilePath);
    simplecsv::outputr(ofs,{"key","value"});
    for (auto & s : mSettings)
        simplecsv::outputr(ofs,{s.first,s.second});    
}