#include <filesystem>

#include "htmlcsvwriter.h"
#include "simplecsv.h"
#include "settings.h"
#include "utils.h"
#include "command.h"
#include "colours.h"

simpleDataCSV::simpleDataCSV(std::string name) : mStream(getOutputPath_Jekyll() + "_data/" + name + ".csv")
{
    if (!mStream.is_open())
        TERMINATE("Unable to open file " + getOutputPath_Jekyll() + "_data/" + name + ".csv for writing.");
}
simpleDataCSV::~simpleDataCSV()
{
    mStream.close();
}

void simpleDataCSV::addrow(const std::vector<std::string> &row)
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
    recreate_Directory(getOutputPath_Html());
    recreate_Directory(getOutputPath_Jekyll());

    CopyHTMLFolder();
    write_basevars(s);
    write_projectbacklog_csv(s);
    write_projectgantt_csv(s);
    write_peopleeffortbymonth_months_people_csvs(s);
    write_dashboard(s);

    run_jekyll();

    copy_site();
}

void HTMLCSVWriter::CopyHTMLFolder() const
{ // ---------------------------

    // also copy across all support_files into the HTML directory.
    {
        std::string html = getOutputPath_Jekyll();
        std::string opt_debug = getExePath() + "includes/dpkg_include/opt/jpf/html";
        std::string opt_local = getLocalTemplatePath();
        std::string opt_system = getOptHTMLPath();

        std::string opt = opt_system;
        if (std::filesystem::exists(opt_local))
        {
            loginfo("Using project directory for Jekyll template: "+opt_local);
            opt = opt_local;
        } 
        else if (std::filesystem::exists(opt_debug))
        {
            loginfo("Using local directory for Jekyll files: "+opt_debug);
            opt=opt_debug;
        } 

        namespace fs = std::filesystem;
        // recursive copy getOptHTMLPath to getOutputPath_Html
        fs::copy(opt, html, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

        std::string ganttpath = html + "static/gantt"; // one of the paths we expect to be present.
        if (!fs::exists(ganttpath))
            fatal("HTML contribution directory was not successfully created: " + ganttpath);

        logdebug(S() << "Copied HTML template from " << opt);
    }
    // ----------------------------
}

void HTMLCSVWriter::copy_site() const
{
    namespace fs = std::filesystem;
    fs::copy(getOutputPath_Jekyll() + "_site", getOutputPath_Html(), fs::copy_options::recursive | fs::copy_options::overwrite_existing);
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
                "Blocked"});

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

void HTMLCSVWriter::write_projectgantt_csv(const scheduler::scheduler &s) const
{
    simpleDataCSV csv("projectgantt");

    csv.addrow({"TaskID",
                "TaskName",
                "Start",
                "End",
                "Colour"});

    std::vector<scheduler::rgbcolour> Colours(s.getProjects().size(), scheduler::rgbcolour({0, 0, 0}));
    colours::ColorGradient heatMapGradient;
    unsigned int ci = 0;
    for (ci = 0; ci < s.getProjects().size(); ++ci)
    {
        float r, g, b;
        float v = (float)ci / (float)(s.getProjects().size());
        heatMapGradient.getColorAtValue(v, r, g, b);
        Colours[ci] = scheduler::rgbcolour{.r = (int)(r * 255.0 + 0.5), .g = (int)(g * 255.0 + 0.5), .b = (int)(b * 255.0 + 0.5)};
    }

    int i = 0;
    for (auto &p : s.getProjects())
    {
        std::string colour = S() << "rgb(" << Colours[i].r << ", " << Colours[i].g << ", " << Colours[i].b << ")";

        ++i;

        csv.addrow({S() << "Task" << i, // first task is Task1
                    p.getId(),
                    p.mActualStart.getYYYY_MM_DD(),
                    p.mActualEnd.getYYYY_MM_DD(),
                    colour});
    }
}

void HTMLCSVWriter::write_basevars(const scheduler::scheduler &s) const
{
    simpleDataCSV csv("basevars");
    time_t now = time(0);
    // convert now to string form
    char *date_time = ctime(&now);

    csv.addrow({"key", "value"});
    csv.addrow({"version", gSettings().getJPFFullVersionStr()});
    csv.addrow({"timedate", date_time});
    csv.addrow({"maxmonth", S() << get_maxmonth(s)});
}

void HTMLCSVWriter::run_jekyll() const
{
    timer tmr;

    std::string jekyllpath;
    if (std::filesystem::exists("/usr/local/bin/jekyll")) // prefer /usr/local/bin - that's where the latest gets installed.
        jekyllpath="/usr/local/bin/jekyll";
    else if (std::filesystem::exists("/usr/bin/jekyll"))
        jekyllpath="/usr/bin/jekyll";
    else
        jekyllpath="jekyll"; 

    loginfo("Running Jekyll build on " + getOutputPath_Jekyll());
    std::string cmd = "cd " + getOutputPath_Jekyll() + " ; "+jekyllpath+" b 2>&1";

    raymii::Command c;

    raymii::CommandResult r = c.exec(cmd);

    if (r.exitstatus != 0)
        throw(TerminateRunException(r.output));
    loginfo(S() << "Jekyll Finished in " << tmr.stop() << " ms.");
}

void HTMLCSVWriter::write_peopleeffortbymonth_months_people_csvs(const scheduler::scheduler &s) const
{
    simpleDataCSV csv("peopleeffortbymonth");

    csv.addrow(
        {"PersonCode",
         "PersonName",
         "MonthNum",
         "DateStr",
         "ProjectName",
         "ProjectColour",
         "ProjectId",
         "Effort"});

    for (scheduler::tNdx personNdx = 0; personNdx < s.getPeople().size(); ++personNdx)
    {

        std::vector<std::vector<double>> DevDaysTally;
        std::vector<scheduler::tProjectInfo> ProjectInfo;
        s.CalculateDevDaysTally(DevDaysTally, ProjectInfo, personNdx);
        if (DevDaysTally.size() == 0)
            return; // no data.

        std::string pCode = s.getPeople()[personNdx].mName;
        removewhitespace(pCode);
        unsigned long maxmonth = DevDaysTally[0].size();

        for (unsigned int i = 0; i < ProjectInfo.size(); ++i)
            for (unsigned int m = 0; m < maxmonth; ++m)
            {
                auto &c = ProjectInfo[i].mColour;
                std::string colour = S() << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")";

                csv.addrow(
                    {pCode,
                     s.getPeople()[personNdx].mName,
                     S() << m,
                     monthIndex(m).getString(),
                     ProjectInfo[i].mName,
                     colour,
                     ProjectInfo[i].mId,
                     S() << std::fixed << std::setprecision(1) << DevDaysTally[i][m]});
            }
    }

    {
        simpleDataCSV csv2("months");
        csv2.addrow({"MonthNum", "DateStr"});

        unsigned long maxmonth = get_maxmonth(s);
        for (unsigned int m = 0; m < maxmonth; ++m)
            csv2.addrow({S() << m, monthIndex(m).getString()});
    }

    {
        simpleDataCSV csv3("people");
        csv3.addrow({"PersonCode", "PersonName"});

        for (auto p : s.getPeople())
        {
            std::string pCode = p.mName;
            removewhitespace(pCode);
            csv3.addrow({pCode, p.mName});
        }
    }

    {
        simpleDataCSV csv4("projects");
        csv4.addrow({"ProjectId", "ProjectName", "ProjectColour", "ProjectType"});

        std::vector<scheduler::tProjectInfo> pi;
        s.getProjectExtraInfo(pi);

        for (auto p : pi)
        {
            auto &c = p.mColour;
            std::string colour = S() << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")";
            csv4.addrow({p.mId,
                         p.mName,
                         colour,
                         s.ItemType2String(p.mType)});
        }
    }
}

unsigned long HTMLCSVWriter::get_maxmonth(const scheduler::scheduler &s) const
{
    // determine the maximum month to consider.
    unsigned long maxmonth = 0;
    {
        for (auto &z : s.getItems())
            if (z.mActualEnd.getMonthIndex() >= maxmonth)
                maxmonth = z.mActualEnd.getMonthIndex() + 1;

        maxmonth = std::min(maxmonth, gSettings().endMonth() + 1);
    }
    return maxmonth;
}

void HTMLCSVWriter::write_dashboard(const scheduler::scheduler &s) const
{
    std::vector<std::vector<double>> DevDaysTally;
    std::vector<scheduler::tProjectInfo> ProjectInfo;
    s.CalculateDevDaysTally(DevDaysTally, ProjectInfo);
    ASSERT(DevDaysTally.size() > 0);
    if (DevDaysTally.size() == 0)
        return; // no data.
    unsigned long maxmonth = DevDaysTally[0].size();

    { // by month total cost graph
        simpleDataCSV csv("projectcostbymonth");

        csv.addrow(
            {"MonthNum",
             "DateStr",
             "ProjectName",
             "ProjectColour",
             "ProjectId",
             "ProjectCost"});

        for (unsigned int i = 0; i < DevDaysTally.size(); ++i)
            for (unsigned int m = 0; m < maxmonth; ++m)
            {
                auto &c = ProjectInfo[i].mColour;
                csv.addrow({S() << m,
                            monthIndex(m).getString(),
                            ProjectInfo[i].mName,
                            S() << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")",
                            ProjectInfo[i].mId,
                            S() << (int)(0.5 + gSettings().dailyDevCost() * DevDaysTally[i][m])});
            }
    }

    // -----------------------

    { // total project cost pie graph
        std::vector<double> vProjectCostRemaining(DevDaysTally.size(), 0.0);
        for (unsigned int i = 0; i < DevDaysTally.size(); ++i)
            for (double j : DevDaysTally[i])
                vProjectCostRemaining[i] += j;

        double totalProjectCostRemaining = 0.0;
        for (auto &vpc : vProjectCostRemaining)
            totalProjectCostRemaining += vpc;

        simpleDataCSV csv2("projectcosttotal");

        csv2.addrow(
            {"ProjectId",
             "ProjectName",
             "ProjectColour",
             "TextLabel",
             "Cost"});

        for (unsigned int i = 0; i < ProjectInfo.size(); ++i)
        {
            std::string textlabel; // only significant projects get a label!
            if (vProjectCostRemaining[i] > 0.01 * totalProjectCostRemaining)
                textlabel = S() << ProjectInfo[i].mId << "   " << getDollars(gSettings().dailyDevCost() * vProjectCostRemaining[i]);

            auto &c = ProjectInfo[i].mColour;
            csv2.addrow(
                {ProjectInfo[i].mId,
                 ProjectInfo[i].mName,
                 S() << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")",
                 textlabel,
                 S() << (int)(0.5 + gSettings().dailyDevCost() * vProjectCostRemaining[i])});
        }
    }

    // -----------------------

    { // BAU!
        using namespace scheduler;

        simpleDataCSV csv4("projecttypes");
        csv4.addrow({"TypeNum",
                     "TypeName",
                     "TypeColour"});
        for (unsigned int i = 0; i < kNumItemTypes; ++i)
        {
            rgbcolour c;
            switch (i)
            {
            case kBAU:
                c = {190, 30, 50};
                break;
            case kNew:
                c = {30, 190, 30};
                break;
            case kUna:
                c = ProjectInfo[ProjectInfo.size() - 3].mColour;
                break;
            case kHol:
                c = ProjectInfo[ProjectInfo.size() - 4].mColour;
                break;
            };
            csv4.addrow(
                {S() << i,
                 s.ItemType2String(static_cast<tItemTypes>(i)),
                 S() << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")"});
        }

        simpleDataCSV csv3("projecttypepercents");

        csv3.addrow({"TypeNum",
                     "MonthNum",
                     "DateStr",
                     "Percentage",
                     "Label"});

        std::vector<std::vector<double>> DDT; // [BAU/New][month]
        DDT.resize(kNumItemTypes);
        for (unsigned int i = 0; i < kNumItemTypes; ++i)
            DDT[i].resize(maxmonth, 0.0);

        for (unsigned int pi = 0; pi < DevDaysTally.size(); ++pi)
            for (unsigned int m = 0; m < maxmonth; ++m)
                DDT[ProjectInfo[pi].mType][m] += DevDaysTally[pi][m];

        for (unsigned int m = 0; m < maxmonth; ++m)
        { // make percentages.
            double tot = DDT[kBAU][m] + DDT[kNew][m] + DDT[kUna][m] + DDT[kHol][m];
            DDT[kBAU][m] = (DDT[kBAU][m] * 100) / tot;
            DDT[kNew][m] = (DDT[kNew][m] * 100) / tot;
            DDT[kHol][m] = (DDT[kHol][m] * 100) / tot;
            DDT[kUna][m] = 100 - DDT[kBAU][m] - DDT[kNew][m] - DDT[kHol][m];
        }

        for (unsigned int i = 0; i < kNumItemTypes; ++i)
            for (unsigned int m = 0; m < maxmonth; ++m)
            {

                csv3.addrow(
                    {
                        S() << i,
                        S() << m,
                        monthIndex(m).getString(),
                        S() << std::setprecision(3) << DDT[i][m],
                        S() << s.ItemType2String(static_cast<tItemTypes>(i)) 
                                << " - " << (int)(0.5 + DDT[i][m]) << "%"
                    });
            }
    }
}
