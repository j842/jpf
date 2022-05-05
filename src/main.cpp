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

void run_common(backlog & b)
{
    create_output_directories();

    std::ofstream ofs_backlog(getOutputPath_Txt()+"backlog.txt");
    ofs_backlog<<"Project backlog starting from "<< itemdate::date2strNice(gSettings().startDate()) << std::endl;
    b.displaybacklog(ofs_backlog);
    ofs_backlog.close();

    std::ofstream ofs_people(getOutputPath_Txt()+"people.txt");
    b.displaypeople(ofs_people);
    ofs_people.close();

    std::ofstream ofs_ganttcsv(getOutputPath_Csv()+"gantt.csv");
    b.save_gantt_project_file(ofs_ganttcsv);
    ofs_ganttcsv.close();

    std::ofstream ofs_milestones(getOutputPath_Txt()+"milestones.txt");
    b.displaymilestones(ofs_milestones);
    ofs_milestones.close();

    std::ofstream ofs_raw(getOutputPath_Txt()+"raw_backlog.txt");
    b.displaybacklog_raw(ofs_raw);
    ofs_raw.close();

    std::ofstream ofs_projects(getOutputPath_Txt()+"projects.txt");
    b.displayprojects(ofs_projects);
    ofs_projects.close();

    // output HTML...
    std::ofstream indexhtml(getOutputPath_Html()+"index.html");
    b.outputHTML_Index(indexhtml);
    indexhtml.close();

    std::ofstream peoplehtml(getOutputPath_Html()+"people.html");
    b.outputHTML_People(peoplehtml);
    peoplehtml.close();

    std::ofstream dashboardhtml(getOutputPath_Html()+"costdashboard.html");
    b.outputHTML_Dashboard(dashboardhtml);
    dashboardhtml.close();

    std::ofstream biggantthtml(getOutputPath_Html()+"highlevelgantt.html");
    b.outputHTML_High_Level_Gantt(biggantthtml);
    biggantthtml.close();

    std::ofstream detailedgantthtml(getOutputPath_Html()+"detailedgantt.html");
    b.outputHTML_Detailed_Gantt(detailedgantthtml);
    detailedgantthtml.close();    

    std::ofstream rawbackloghtml(getOutputPath_Html()+"rawbacklog.html");
    b.outputHTML_RawBacklog(rawbackloghtml);
    rawbackloghtml.close();    
}

void run_console()
{
    timer tmr;

    try
    {
        projects p;
        teams t;
        backlog b(p,t);
        b.schedule();
        std::cout << std::endl << std::endl;
        b.displayprojects(std::cout);
        run_common(b);        

        std::cout << std::endl << "Completed in "<<std::setprecision(3) << tmr.stop() <<"ms."<<std::endl;
    }
    catch(TerminateRunException& pEx)
    {
        std::cerr << pEx.what() << std::endl;
    }

}


void run_watch(int port)
{
    webserver wserver(port);
    watcher w(getInputPath());

    while (true)
    {
        try 
        {
            projects p;
            teams t;
            backlog b(p,t);
            b.schedule();

            timer tmr;
            run_common(b);
            std::cout << "File output done in "<<std::setprecision(3)<<tmr.stop()<<"ms."<<std::endl;
        }
        catch (TerminateRunException& pEx)
        {
            std::vector<std::string> htmlfiles={"index.html","people.html","costdashboard.html","highlevelgantt.html","detailedgantt.html","rawbacklog.html"};
            for (auto & x : htmlfiles)
                backlog::outputHTMLError(getOutputPath_Html()+x, pEx.what());
            std::cerr << pEx.what() << std::endl;
        }

        std::cout<<std::endl<<"Watching for changes. Ctrl-c to exit."<<std::endl;
        w.waitforchange();
        std::cout<<"Files updated...recalculating!"<<std::endl;
    }
}

void showhelp()
{
    std::cout << std::endl;
    std::cout << "jpf "<< gSettings().getJPFVersionStr()<<
                        "-"<<gSettings().getJPFReleaseStr()<< " is a simple auto-balancing forecasting tool for projects across multiple teams.";

    std::cout << R"(

The directory used needs to contain an input folder, with appropriate csv files in it.

usage: jpf [ options ] DIRECTORY

Options:
    -w, -watch      Watch the folder for changes, and update all output files as needed.
                    Also starts a webserver on PORT, displaying the HTML output files.
                    The corresponding webpages will auto-update as inputs change.

    -port=PORT      Change the webserver port to the one specified.

    -c, -create     Create a skeleton working tree in DIRECTORY, which includes input and 
                    output directories, with example input files. 
                    Cannot be used with other options.

    -t, -test       Run unit tests.
                
)";
}

int getport(std::string s)
{
    int port=5000;
    for (unsigned int i=0;i<s.length();++i)
        if (s[i]=='=' && i<s.length()-1)
        {
            port=atoi(s.c_str()+i+1);
        }

    if (port<=1024)
        TERMINATE("Need user settable port - i.e. > 1024.");
    return port;
}

void checkcreate(std::string d)
{
    if (!std::filesystem::exists(d))
    {
        if (!std::filesystem::create_directory(d))
            TERMINATE("Could not create directory: "+d);
        std::cout<<"Created directory: "<<d<<std::endl;
    }    
}

void create_output_directories()
{
    std::string po =  gSettings().getRoot()+"/output/";
    checkcreate(po);
    checkcreate(getOutputPath_Txt());
    checkcreate(getOutputPath_Csv());
    checkcreate(getOutputPath_Html());
}

void create_directories()
{
    std::string pr = gSettings().getRoot();
    std::string pi = pr+"/input/";
    std::string ex = "/opt/jpf/input/";

    if (std::filesystem::exists(pi))
        TERMINATE("Can't create directories - input dir already exists: "+pi);

    checkcreate(pr);
    create_output_directories();

    if (!std::filesystem::exists(ex))
        TERMINATE("Expected example files were not installed in /opt/jpf/input.");

    copy("/opt/jpf/input/", pr, std::filesystem::copy_options::recursive);
    if (!std::filesystem::exists(pi))
        TERMINATE("Input directory was not successfully created: "+pi);
}

void runtests()
{
    CPPUNIT_NS::Test *suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( suite );
    bool wasSucessful = runner.run();

    std::cout << ( wasSucessful ? "Success!!" : "Fail!!" )<< std::endl;
}

int main(int argc, char **argv)
{    
    if (argc<=1)
    {
        showhelp();
        exit(0);
    }
    if (iSame(argv[1],"-t") || iSame(argv[1],"-test"))
        {runtests(); return 0;}

    catch_ctrl_c();

    try
    {
        bool watch=false, create=false;
        int port=5000;

        std::vector<std::string> params;
        for (int i=1;i<argc-1;++i)
            params.push_back(argv[i]);

        std::string directory(argv[argc-1]);

        for (auto & s : params)
        {
            if (s.length()<2) TERMINATE("Bad parameter: " + s);
            if (s[0]!='-')  TERMINATE("Options must start with - : " + s);
            switch (s[1]) {
                case 'w' : watch=true; break;
                case 'p' : port = getport(s); break;
                case 'c' : create = true; if (params.size()>1) TERMINATE("Create option must be specified on its own."); break;
                default: TERMINATE("Bad parameter: " + s);
            }
        }               
        gSettings().setRoot(directory);

        std::cout << std::endl;
        std::cout << "John's Project Forecaster "<<
                        gSettings().getJPFVersionStr()<<
                        "-"<<gSettings().getJPFReleaseStr()<<
                        " - An auto-balancing forecasting tool." <<std::endl<<std::endl;
        std::cout << "Root directory: "<<gSettings().getRoot()<<std::endl;

        if (create)
            create_directories();
        else
        {
            gSettings().load_settings();
            if (!gSettings().RootExists())
                TERMINATE("Root directory "+gSettings().getRoot()+" does not exist.");

            if (watch)
                run_watch(port);
            else
                run_console();
        }
    }
    catch(const ctrlcException& e)
    {
        std::cerr << "\n\n" << e.what() << '\n';
    }
    catch(const TerminateRunException& e)
    {
        std::cerr << "\n\n" << e.what() << '\n';
    }

    return 0;
}
