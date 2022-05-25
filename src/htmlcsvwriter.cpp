#include <filesystem>

#include "htmlcsvwriter.h"
#include "simplecsv.h"
#include "settings.h"
#include "utils.h"
#include "command.h"
#include "colours.h"

simpleDataCSV::simpleDataCSV(std::string name) : mStream(getOutputPath_Jekyll() + "_data/" + name +".csv")
{
    if (!mStream.is_open())
        TERMINATE("Unable to open file " + getOutputPath_Jekyll() + "_data/" + name + ".csv for writing.");
}
simpleDataCSV::~simpleDataCSV()
{
    mStream.close();
}

void simpleDataCSV::addrow(const std::vector<std::string> & row)
{
    simplecsv::outputr(mStream, row);
}



HTMLCSVWriter::HTMLCSVWriter()
{
}

void HTMLCSVWriter::recreate_Directory(std::string path) const
{
    if (std::filesystem::exists(path))
        std::filesystem::remove_all(path);
    if (!std::filesystem::create_directory(path))
        TERMINATE(S() << "Could not create directory: " << path);
    logdebug(S() << "Created directory: " << path);
}


void HTMLCSVWriter::createHTMLFolder(const scheduler::scheduler &s) const
{
    recreate_Directory(getOutputPath_Jekyll());
    recreate_Directory(getOutputPath_Html());

    CopyHTMLFolder();
    write_basevars(s);
    write_projectbacklog_csv(s);
    write_projectgantt_csv(s);

    run_jekyll();

    copy_site();
}

void HTMLCSVWriter::CopyHTMLFolder() const
{    // ---------------------------

    // also copy across all support_files into the HTML directory.
    {
        std::string html = getOutputPath_Jekyll();
        std::string opt1 = getLocalTemplatePath();
        std::string opt2 = getOptHTMLPath();

        std::string opt = opt2;
        if (std::filesystem::exists(opt1))
            opt = opt1;

        namespace fs = std::filesystem;
        // recursive copy getOptHTMLPath to getOutputPath_Html
        fs::copy(opt, html, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

        std::string ganttpath = html + "_site/static/gantt"; // one of the paths we expect to be present.
        if (!fs::exists(ganttpath))
            fatal("HTML contribution directory was not successfully created: " + ganttpath);

        logdebug(S()<<"Copied HTML template from "<<opt);
    }
    // ----------------------------
}

void HTMLCSVWriter::copy_site() const
{
    namespace fs = std::filesystem;
    fs::copy(getOutputPath_Jekyll()+"_site", getOutputPath_Html(), fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}


void HTMLCSVWriter::write_projectbacklog_csv(const scheduler::scheduler &s) const
{
    simpleDataCSV csv("projectbacklog");

    std::vector<int> v_sorted;
    s.prioritySortArray(v_sorted);

    csv.addrow({"Project",
                "Start",
                "End",
                "Task Name",
                "Team",
                "Blocked"
                });

    for (auto &x : v_sorted)
    {
        auto &z = s.getItems()[x];

        csv.addrow({s.getProjects()[z.mProject].getName(),
                    z.mActualStart.getStr_nice_short(),
                    z.mActualEnd.getStr_nice_short(),
                    z.mDescription,
                    s.getInputs().mT.at(z.mTeamNdx).mId,
                    z.mBlockedBy});
    }
}

struct rgbcolour
{
    int r, g, b;
};

void HTMLCSVWriter::write_projectgantt_csv(const scheduler::scheduler &s) const
{
    simpleDataCSV csv("projectgantt");    

    csv.addrow({
        "TaskID",
        "TaskName",
        "Start",
        "End",
        "Colour"
    });

    std::vector<rgbcolour> Colours( s.getProjects().size(), rgbcolour({0,0,0}));
    colours::ColorGradient heatMapGradient;
    unsigned int ci = 0;
    for (ci = 0; ci < s.getProjects().size(); ++ci)
    {
        float r, g, b;
        float v = (float)ci / (float)(s.getProjects().size());
        heatMapGradient.getColorAtValue(v, r, g, b);
        Colours[ci] = rgbcolour{.r = (int)(r * 255.0 + 0.5), .g = (int)(g * 255.0 + 0.5), .b = (int)(b * 255.0 + 0.5)};
    }

    int i=0;
    for (auto & p : s.getProjects())
    {        
        std::string colour = S() << "rgb("<<Colours[i].r <<", "<<Colours[i].g<<", "<<Colours[i].b<<")";

        ++i;

        csv.addrow({
            S()<<"Task"<< i, // first task is Task1
            p.getId(),
            p.mActualStart.getYYYY_MM_DD(),
            p.mActualEnd.getYYYY_MM_DD(),
            colour
        });
    }
}


void HTMLCSVWriter::write_basevars(const scheduler::scheduler & s) const
{
    simpleDataCSV csv("basevars");
    time_t now = time(0);
    // convert now to string form
    char* date_time = ctime(&now);

    csv.addrow({"key","value"});
    csv.addrow({"version", gSettings().getJPFFullVersionStr()});
    csv.addrow({"timedate", date_time });
}


void HTMLCSVWriter::run_jekyll() const
{
    timer tmr;

    loginfo("Running Jekyll build on "+getOutputPath_Jekyll());
    std::string cmd = "cd "+getOutputPath_Jekyll()+" ; /usr/local/bin/jekyll b 2>&1";

    raymii::Command c;

    raymii::CommandResult r=c.exec(cmd);
    
    if (r.exitstatus!=0)
        throw (TerminateRunException(r.output));
    loginfo(S()<<"Jekyll Finished in "<<tmr.stop()<<" ms.");
}
