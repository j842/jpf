#include <climits>
#include <strings.h>
#include <dirent.h>
#include <errno.h>
#include <filesystem>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "settings.h"
#include "simplecsv.h"
#include "utils.h"
#include "workdate.h"
#include "version.h"

settings::settings() : mLoaded(false), mRootDir(),
                       mOutputModeHTML(false),
                       mValidSettings({"startdate", "enddate", "inputversion", "costperdevday", "title", "loglevel"})
{
}

void settings::setSettings(simpledate startDate, simpledate endDate)
{
    mStartDate = startDate;
    mEndDate = endDate;
    mLoaded = true;
    mRootDir = "";
}

void settings::load_settings()
{
    std::map<std::string, std::string> settings;
    simplecsv c("settings.csv");

    if (!c.openedOkay())
        TERMINATE(S() << "Could not open " << getInputPath() << "settings.csv.");

    std::vector<std::string> row;
    while (c.getline(row, 3))
    {
        if (isValid(row[0]))
            settings[row[0]] = row[1];
        else
            logwarning(S() << "Unrecognised setting in settings.csv:  " << row[0]);
    }

    mLoaded = true;

    // can't use itemdate yet, as mStartDate not set.
    std::string sdate = getSettingS("startdate", settings);
    if (sdate.length() == 0 || iSame(sdate, "today"))
        mStartDate = workdate::snapWorkDay_forward(boost::gregorian::day_clock::local_day());
    else
        mStartDate = workdate::snapWorkDay_forward(simpledate(getSettingS("startdate", settings)));

    std::string edate = getSettingS("enddate", settings);
    if (sdate.length() == 0 || iSame(sdate, "forever"))
        mEndDate = workdate(boost::gregorian::date(mStartDate.getGregorian().year(), 12, 1)); // dec same year. <shrug>
    else
        mEndDate = workdate(simpledate(getSettingS("enddate", settings)));

    ASSERT(!simpledate::isWeekend(mStartDate));
    ASSERT(!simpledate::isWeekend(mEndDate));

    mMinLogLevel = kLINFO;
    std::vector<std::string> loglevels = {"debug", "info", "warning", "error"};
    for (unsigned int ll = 0; ll < loglevels.size(); ++ll)
        if (iSame(getSettingS("loglevel", settings), loglevels[ll]))
            mMinLogLevel = static_cast<eLogLevel>(ll);

    mRequiredInputVersion = getSettingI("inputversion", settings);
    mDailyDevCost = getSettingD("costperdevday", settings);

    mTitle = getSettingS("title", settings);
    if (mTitle.length() == 0)
        mTitle = "John's Project Forecaster";

    if (getInputVersion() < getRequiredInputVersion())
        TERMINATE("The input files being used require a newer version of jpf.");
    if (getInputVersion() > getRequiredInputVersion())
        TERMINATE(S() << "The input files need to be updated to support the current version of jpf."
                      << "\nUpdate inputversion in settings.csv to " << getInputVersion() << " when done.");
}

void settings::save_settings_CSV(std::ostream &os) const
{
    os << "Setting Name,Value,Description" << std::endl;
    std::vector<std::string> row;

    // mValidSettings({"startdate","enddate","inputversion","costperdevday","title","loglevel"})
    simplecsv::outputr(os, {"inputversion", S() << mRequiredInputVersion, "Input file format version"});
    simplecsv::outputr(os, {"startdate", mStartDate.getStr(), "Start of scheduled period"});
    simplecsv::outputr(os, {"enddate", mEndDate.getStr(), "Last month to display in monthly graphs"});
    simplecsv::outputr(os, {"costperdevday", S() << mDailyDevCost, "Cost per developer per day (including overheads)"});
    simplecsv::outputr(os, {"title", mTitle, "Title for reports"});
    simplecsv::outputr(os, {"loglevel", levelname(mMinLogLevel), "Logging level (Debug, Info, Warning, or Error)"});
}

std::string settings::getTitle() const
{
    return mTitle;
}

std::string settings::getSettingS(std::string settingName, const std::map<std::string, std::string> &settings) const
{
    ASSERT(mLoaded);
    auto pos = settings.find(settingName);
    if (pos == settings.end())
    {
        TERMINATE(S() << "Setting \"" << settingName << "\" is not defined in settings.csv.");
    }
    return pos->second;
}

int settings::getSettingI(std::string settingName, const std::map<std::string, std::string> &settings) const
{
    ASSERT(mLoaded);
    return atoi(getSettingS(settingName, settings).c_str());
}

double settings::getSettingD(std::string settingName, const std::map<std::string, std::string> &settings) const
{
    ASSERT(mLoaded);
    return strtod(getSettingS(settingName, settings).c_str(), NULL);
}

simpledate settings::startDate() const
{
    ASSERT(mLoaded);
    return mStartDate;
}

simpledate settings::endDate() const
{
    ASSERT(mLoaded);
    return mEndDate;
}
monthIndex settings::endMonth() const
{
    ASSERT(mLoaded);
    return monthIndex(endDate());
}

double settings::dailyDevCost() const
{
    ASSERT(mLoaded);
    return mDailyDevCost;
}

settings &gSettings()
{
    static settings sSettings;
    return sSettings;
}

// static
int settings::getInputVersion()
{
    int v = __INPUT_VERSION;
    return v;
}

// static
std::string settings::getInputVersionStr()
{
    std::ostringstream oss;
    oss << getInputVersion();
    return oss.str();
}

int settings::getRequiredInputVersion() const
{
    ASSERT(mLoaded);
    return mRequiredInputVersion;
}

std::string settings::getJPFVersionStr()
{
    return __JPF_VERSION;
}

std::string settings::getJPFReleaseStr()
{
    return __JPF_RELEASE;
}

std::string settings::getJPFFullVersionStr()
{
    return S() << getJPFVersionStr() << "-" << getJPFReleaseStr();
}

eLogLevel settings::getMinLogLevel() const
{
    return mMinLogLevel;
}

void settings::setMinLogLevel(eLogLevel l)
{
    mMinLogLevel = l;
}

void settings::setRoot(std::string path)
{
    if (path.length() == 0)
        TERMINATE("Empty path when trying to set root directory.");

    mRootDir = makecanonicalslash(path);
}

std::string settings::getRoot() const
{
    return mRootDir;
}

bool settings::RootExists() const
{
    ASSERT(mRootDir.length() > 0);
    DIR *dir = opendir(getRoot().c_str());
    if (dir)
    {
        /* Directory exists. */
        closedir(dir);
        return true;
    }
    else if (ENOENT == errno)
    {
        /* Directory does not exist. */
        return false;
    }
    else
    {
        /* opendir() failed for some other reason. */
        return false;
    }
}

bool settings::isValid(std::string key) const
{
    for (auto &i : mValidSettings)
        if (iSame(key, i))
            return true;
    return false;
}

void settings::advance(workdate newStart)
{
    mStartDate = newStart.getGregorian();

    if (mEndDate <= mStartDate)
        mEndDate = boost::gregorian::date(mStartDate.getGregorian().year() + 1, 1, 1);
}

const std::string getInputPath() { return gSettings().getRoot() + "input/"; }
const std::string getOutputPath_Base() { return gSettings().getRoot() + "output/"; }
const std::string getOutputPath_Txt() { return gSettings().getRoot() + "output/txt/"; }
const std::string getOutputPath_Html() { return gSettings().getRoot() + "output/html/"; }
const std::string getOutputPath_Jekyll() { return gSettings().getRoot() + "output/.jekyll/"; }
const std::string getOutputPath_Csv() { return gSettings().getRoot() + "output/csv/"; }
const std::string getOutputPath_Log() { return gSettings().getRoot() + "output/log/"; }
const std::string getOutputPath_PDF() { return gSettings().getRoot() + "output/pdf/";}
const std::string getLocalTemplatePath() { return gSettings().getRoot() + "template/"; }

const std::string getExePath()
{
    std::vector<char> buf(400);
    size_t len;

    do
    {
        buf.resize(buf.size() + 100);
        len = ::readlink("/proc/self/exe", &(buf[0]), buf.size());
    } while (buf.size() == len);

    if (len > 0)
    {
        buf[len] = '\0';
        std::string exepath(&(buf[0]));
        exepath = std::filesystem::canonical(exepath);
        if (exepath.length() == 0)
            return "";

        unsigned int i = exepath.length() - 1;
        while (i > 0 && exepath[i] != '/')
            --i;
        if (i > 0 && i < exepath.length() - 1)
            exepath.erase(exepath.begin() + i + 1, exepath.end()); // keep ye slash.
        return exepath;
    }
    /* handle error */
    return "";
}

const std::string getInputPath_Jekyll()
{
    std::string opt_local = getLocalTemplatePath();
    if (std::filesystem::exists(opt_local))
        return opt_local;

    std::string opt_system = "/opt/jpf/html/";
    if (std::filesystem::exists(opt_system))
        return opt_system;

    TERMINATE(S()<<"Could not find Template directory at "<<opt_local);
    return "";
}

const std::string getHomeDir()
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    return makecanonicalslash(homedir);
}

void settings::setOutputModeHTML(bool output_as_HTML)
{
    mOutputModeHTML = output_as_HTML;
}

bool settings::getOutputModeHTML() const
{
    return mOutputModeHTML;
}
