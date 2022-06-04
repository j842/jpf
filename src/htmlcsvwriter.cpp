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

    std::vector<std::vector<double>> DevDaysTally;
    std::vector<scheduler::tProjectInfo> ProjectInfo;
    s.CalculateDevDaysTally(DevDaysTally, ProjectInfo);

    write_basevars(s);
    write_projectbacklog_csv(s);
    write_projectgantt_csv(s);
    write_peopleeffortbymonth_months_people_csvs(s);
    write_peoplebacklog(s);
    write_settings(s);
    write_projects_csv(s);

    // dashboard.html
    write_projecttypes(s, DevDaysTally, ProjectInfo);
    write_projectcostbymonth(s, DevDaysTally, ProjectInfo);
    write_projectcosttotal(s, DevDaysTally, ProjectInfo);
    write_projecttypepercents(s, DevDaysTally, ProjectInfo);

    write_all_tag_files(s);

    run_jekyll();

    copy_site();
}


/* static */ void HTMLCSVWriter::CopyHTMLFolder() const
{ // ---------------------------

    // also copy across all support_files into the HTML directory.
    {
        std::string html = getOutputPath_Jekyll();
        std::string opt = getInputPath_Jekyll();
        loginfo("Using this directory for Jekyll files: " + opt);

        namespace fs = std::filesystem;
        // recursive copy to getOutputPath_Html
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

    std::vector<scheduler::tProjectInfo> ProjectInfo;
    s.getProjectExtraInfo(ProjectInfo);

    std::vector<scheduler::rgbcolour> Colours(s.getProjects().size(), scheduler::rgbcolour({0, 0, 0}));

    csv.addrow({"projectindex",
                "project",
                "projectcolour",
                "start",
                "end",
                "taskname",
                "team",
                "id",
                "blocked",
                "dependencies",
                "tags",
                "devdays",
                "resources",
                "comments"});

    for (auto &x : v_sorted)
    {
        auto &z = s.getItems()[x];

        std::string dependencies;
        for (auto &d : z.mDependencies)
            dependencies += d + " ";
        std::string tags;
        for (auto &t : z.mTags)
            tags += t + " ";
        std::string devdays = S() << std::fixed << std::setprecision(2) << 0.01 * (double)z.mDevCentiDays;
        std::string resources;
        for (unsigned int pi = 0; pi < z.mResources.size(); pi++)
            resources += S() << z.mResources[pi].mName << ": " << std::fixed << std::setprecision(2) << 0.01 * (double)z.mTotalContribution[pi] << "  ";

        scheduler::rgbcolour rgbc = ProjectInfo[z.mProjectIndex].mColour;

        csv.addrow({S() << z.mProjectIndex,
                    s.getProjects()[z.mProjectIndex].getName(),
                    S() << "rgb(" << rgbc.r << ", " << rgbc.g << ", " << rgbc.b << ")",
                    z.mActualStart.getStr_nice_short(),
                    z.getLastDayWorked().getStr_nice_short(),
                    z.mDescription,
                    s.getInputs().mT.at(z.mTeamNdx).mId,
                    z.mId,
                    z.mBlockedBy,
                    dependencies,
                    tags,
                    devdays,
                    resources,
                    z.mComments});
    }
}

void HTMLCSVWriter::write_all_tag_files(const scheduler::scheduler &s) const
{
    { // tasks
        std::vector<std::string> tagslist;
        for (const auto &i : s.getItems())
            i.addToTags(tagslist);

        {
            simpleDataCSV csv("task_tags");
            csv.addrow({"tag"});
            for (const auto &i : tagslist)
                csv.addrow({i});
        }

        for (auto t : tagslist)
            write_task_tag_file(s, t);
    }

    { // projects
        cTags tagslist;
        for (const auto & i : s.getProjects())
            i.getTags().addToTags(tagslist);

        simpleDataCSV csv("project_tags");
        csv.addrow({"tag"});
        for (const auto & i : tagslist)
            csv.addrow({i});
        csv.addrow({"All_Projects"});

        for (auto t : tagslist)
            write_project_tag_file(s,t);
        write_project_tag_file(s,"All_Projects");
    }
}

void HTMLCSVWriter::write_project_tag_file(const scheduler::scheduler &s, const std::string tag) const
{
    std::vector<scheduler::tProjectInfo> ProjectInfo;
    s.getProjectExtraInfo(ProjectInfo);

    simpleDataCSV csv("project_tag_" + tag);
    csv.addrow({"project",
                "projectcolour",
                "start",
                "end",
                "id",
                "devdays",
                "description",
                "comments",
                "team",
                "projectindex",
                "endparsy"});

    std::vector<std::vector<std::string>> csvcontent;

    for (unsigned int i=0;i< s.getProjects().size();++i)
    {
        auto & z = s.getProjects().at(i);
        if (iSame(tag,"All_Projects") || z.getTags().hasTag(tag))
        { // project has desired tag!
            scheduler::rgbcolour rgbc = ProjectInfo[i].mColour;
            workdate end = z.mActualEnd;
            if (end>z.mActualStart)
                end.decrementWorkDay(); // make closed interval

            cTags team;
            for (auto & itm : s.getItems())
                if (itm.mProjectIndex == i)
                    for (auto & rsc : itm.mResources)
                        if (!team.hasTag(rsc.mName))
                            team.push_back(rsc.mName);

            csvcontent.push_back(
                {
                    z.getName(),
                    S() << "rgb(" << rgbc.r << ", " << rgbc.g << ", " << rgbc.b << ")",
                    z.mActualStart.getStr_nice_short(),
                    end.getStr_nice_short(),
                    z.getId(),
                    S()<<(int)(0.5+(double)z.mTotalDevCentiDays/100.0),
                    z.getDesc(),
                    z.getmComments(),
                    team.getAsString(),
                    S()<<i,
                    end.getStr()
               }
            );
        }
    }

    sort(csvcontent.begin(), csvcontent.end(), [](const auto &lhs, const auto &rhs)
         { return simpledate(lhs[lhs.size() - 1]) < simpledate(rhs[rhs.size() - 1]); });
    for (auto &x : csvcontent)
        csv.addrow(x);
}

void HTMLCSVWriter::write_task_tag_file(const scheduler::scheduler &s, const std::string tag) const
{
    std::vector<scheduler::tProjectInfo> ProjectInfo;
    s.getProjectExtraInfo(ProjectInfo);

    std::vector<scheduler::rgbcolour> Colours(s.getProjects().size(), scheduler::rgbcolour({0, 0, 0}));
    std::vector<std::vector<std::string>> csvcontent;

    for (auto &z : s.getItems())
    {
        bool hasTag(false);
        for (auto &t : z.mTags)
            if (iSame(t, tag))
                hasTag = true;

        if (hasTag)
        {
            std::string dependencies;
            for (auto &d : z.mDependencies)
                dependencies += d + " ";

            std::string devdays = S() << std::fixed << std::setprecision(2) << 0.01 * (double)z.mDevCentiDays;
            scheduler::rgbcolour rgbc = ProjectInfo[z.mProjectIndex].mColour;

            csvcontent.push_back({s.getProjects()[z.mProjectIndex].getName(),
                                  S() << "rgb(" << rgbc.r << ", " << rgbc.g << ", " << rgbc.b << ")",
                                  z.mActualStart.getStr_nice_short(),
                                  z.getLastDayWorked().getStr_nice_short(),
                                  z.mDescription,
                                  s.getInputs().mT.at(z.mTeamNdx).mId,
                                  z.mId,
                                  z.mBlockedBy,
                                  dependencies,
                                  devdays,
                                  z.mComments,
                                  z.getLastDayWorked().getStr()});
        }
    }

    simpleDataCSV csv("task_tag_" + tag);
    csv.addrow({"project",
                "projectcolour",
                "start",
                "end",
                "taskname",
                "team",
                "id",
                "blocked",
                "dependencies",
                "devdays",
                "comments",
                "endparsy"});

    sort(csvcontent.begin(), csvcontent.end(), [](const auto &lhs, const auto &rhs)
         { return simpledate(lhs[lhs.size() - 1]) < simpledate(rhs[rhs.size() - 1]); });

    for (auto &x : csvcontent)
        csv.addrow(x);
}

void HTMLCSVWriter::write_projectgantt_csv(const scheduler::scheduler &s) const
{
    simpleDataCSV csv("projectgantt");

    csv.addrow({"TaskID",
                "TaskName",
                "Start",
                "End",
                "Colour"});

    std::vector<scheduler::tProjectInfo> ProjectInfo;
    s.getProjectExtraInfo(ProjectInfo);

    int i = 0;
    for (auto &p : s.getProjects())
    {
        std::string colour = S() << "rgb(" << ProjectInfo[i].mColour.r << ", " << ProjectInfo[i].mColour.g << ", " << ProjectInfo[i].mColour.b << ")";

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
    std::string ds(date_time);
    trim(ds);

    csv.addrow({"key", "value"});
    csv.addrow({"version", gSettings().getJPFFullVersionStr()});
    csv.addrow({"timedate", ds});
    csv.addrow({"maxmonth", S() << get_maxmonth(s)});
}

void HTMLCSVWriter::run_jekyll() const
{
    timer tmr;

    std::string jekyllpath;
    if (std::filesystem::exists("/usr/local/bin/jekyll")) // prefer /usr/local/bin - that's where the latest gets installed.
        jekyllpath = "/usr/local/bin/jekyll";
    else if (std::filesystem::exists("/usr/bin/jekyll"))
        jekyllpath = "/usr/bin/jekyll";
    else
        jekyllpath = "jekyll";

    loginfo("Running Jekyll build on " + getOutputPath_Jekyll());
    std::string cmd = "cd " + getOutputPath_Jekyll() + " ; " + jekyllpath + " b 2>&1";

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
}

void HTMLCSVWriter::write_projects_csv(const scheduler::scheduler &s) const
{
    simpleDataCSV csv4("projects");
    csv4.addrow({"projectid", "projectname", "projectcolour", "projecttype"});

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

void HTMLCSVWriter::write_projectcosttotal(
    const scheduler::scheduler &s,
    const std::vector<std::vector<double>> &DevDaysTally,
    const std::vector<scheduler::tProjectInfo> &ProjectInfo) const
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

void HTMLCSVWriter::write_projectcostbymonth(
    const scheduler::scheduler &s,
    const std::vector<std::vector<double>> &DevDaysTally,
    const std::vector<scheduler::tProjectInfo> &ProjectInfo) const
{
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
}

void HTMLCSVWriter::write_projecttypes(
    const scheduler::scheduler &s,
    const std::vector<std::vector<double>> &DevDaysTally,
    const std::vector<scheduler::tProjectInfo> &ProjectInfo) const
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
}

void HTMLCSVWriter::write_projecttypepercents(
    const scheduler::scheduler &s,
    const std::vector<std::vector<double>> &DevDaysTally,
    const std::vector<scheduler::tProjectInfo> &ProjectInfo) const
{
    using namespace scheduler;

    if (DevDaysTally.size() == 0)
        return; // no data.
    unsigned long maxmonth = DevDaysTally[0].size();

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
                {S() << i,
                 S() << m,
                 monthIndex(m).getString(),
                 S() << std::setprecision(3) << DDT[i][m],
                 S() << s.ItemType2String(static_cast<tItemTypes>(i))
                     << " - " << (int)(0.5 + DDT[i][m]) << "%"});
        }
}

void HTMLCSVWriter::write_peoplebacklog(const scheduler::scheduler &s) const
{
    simpleDataCSV csv("peoplebacklog");

    csv.addrow({
        "personname","start","end","utilisation","taskname"
    });

    for (auto &z : s.getPeople())
    {
        for (unsigned long in=0;in<s.getItems().size();++in)
        {
            auto & j = s.getItems()[in];
            for (unsigned long pi = 0; pi < j.mResources.size(); ++pi)
            {
                auto &p = j.mResources[pi];
                if (iSame(p.mName, z.mName))
                {
                    tCentiDay utilisation = 100;
                    if (z.getMaxAvialability() > 0)
                        utilisation = j.mLoadingPercent[pi];

                    workdate start( j.mActualStart );
                    workdate end( j.mActualEnd );
                    if (utilisation>0)
                    { // determine the actual start and end date worked for a particular person! tricky.
                        start=j.mActualEnd;
                        end=j.mActualStart;
                        std::string personname = p.mName;
                        workdate s = j.mActualStart;
                        for (;s<j.mActualEnd;s.incrementWorkDay())
                        {
                            for (auto c : z.getChunks(s.getDayAsIndex()))
                                if (c.mItemIndex==in && c.mEffort>0)
                                {
                                    start=std::min(start,s);
                                    workdate s2=s;
                                    s2.incrementWorkDay();
                                    end=std::max(end,s2);
                                }
                        }
                    }
                    
                    if (end>start)
                        end.decrementWorkDay();

                    csv.addrow({
                        p.mName,
                        start.getStr(),
                        end.getStr(),
                        S()<<utilisation,
                        j.getFullName()
                    });
                }
            }
        }
    }
}


void HTMLCSVWriter::write_settings(const scheduler::scheduler & s) const
{
    simpleDataCSV csv("settings");

    csv.addrow({"key","value"});
    csv.addrow({"title",gSettings().getTitle()});
    csv.addrow({"start",gSettings().startDate().getStr()});
    csv.addrow({"end",gSettings().endDate().getStr()});
    csv.addrow({"endmonth",gSettings().endMonth().getString()});
    csv.addrow({"rootdir",gSettings().getRoot()});
}
