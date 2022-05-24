#include <filesystem>

#include "htmlcsvwriter.h"
#include "simplecsv.h"
#include "settings.h"

HTMLCSVWriter::HTMLCSVWriter()
{
}

void HTMLCSVWriter::createHTMLFolder(const scheduler::scheduler &s) const
{
    CopyHTMLFolder();
    write_projectbacklog_csv(s);
}

void HTMLCSVWriter::CopyHTMLFolder() const
{

    if (std::filesystem::exists(getOutputPath_Html()))
        std::filesystem::remove_all(getOutputPath_Html());
    if (!std::filesystem::create_directory(getOutputPath_Html()))
        TERMINATE(S() << "Could not create directory: " << getOutputPath_Html());
    logdebug(S() << "Created directory: " << getOutputPath_Html());

    // ---------------------------

    // also copy across all support_files into the HTML directory.
    {
        std::string html = getOutputPath_Html();
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

void HTMLCSVWriter::write_projectbacklog_csv(const scheduler::scheduler &s) const
{
    std::string projectbacklog_csv = getOutputPath_Html() + "_data/projectbacklog.csv";
    std::vector<int> v_sorted;
    s.prioritySortArray(v_sorted);

    std::ofstream outf(projectbacklog_csv);
    if (!outf.is_open())
        TERMINATE("Unable to open file " + projectbacklog_csv + " for writing.");

    simplecsv::outputr(outf, {"Project",
                             "Team",
                             "Start",
                             "End",
                             "Blocked",
                             "Name"});

    for (auto &x : v_sorted)
    {
        auto &z = s.getItems()[x];

        simplecsv::outputr(outf, {s.getProjects()[z.mProject].getName(),
                                  s.getInputs().mT.at(z.mTeamNdx).mId,
                                  z.mActualStart.getStr(),
                                  z.mActualEnd.getStr(),
                                  z.mBlockedBy,
                                  z.mDescription});
    }

    outf.close();
}