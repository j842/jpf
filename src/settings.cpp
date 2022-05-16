#include <climits>
#include <strings.h>
#include <dirent.h>
#include <errno.h>
#include <filesystem>

#include "settings.h"
#include "simplecsv.h"
#include "utils.h"
#include "workdate.h"
#include "version.h"

settings::settings() : mLoaded(false), mRootDir(),
    mValidSettings({"startdate","enddate","inputversion","costperdevday","title","port"})
{
}

void settings::setSettings( simpledate startDate, simpledate endDate, int port  )
{
    mStartDate = startDate;
    mEndDate = endDate;
    mPort = port;
    mLoaded = true;
    mRootDir = "";
}

void settings::load_settings()
{
    simplecsv c("settings.csv");

    if (!c.openedOkay())
        TERMINATE("Could not open "+getInputPath()+"settings.csv.");

    std::vector<std::string> row;
    while (c.getline(row,3))
    {
        if (isValid(row[0]))
            mSettings[row[0]]=row[1];
        else
            TERMINATE("Unrecognised setting in settings.csv:  "+row[0]);
    }

    mLoaded=true;

    // can't use itemdate yet, as mStartDate not set.
    std::string sdate=getSettingS("startdate");
    if (sdate.length()==0 || iSame(sdate,"today"))
        mStartDate = workdate::snapWorkDay_forward(boost::gregorian::day_clock::local_day());
    else
        mStartDate = workdate::snapWorkDay_forward(simpledate(getSettingS("startdate")));

    std::string edate=getSettingS("enddate");
    if (sdate.length()==0 || iSame(sdate,"forever"))
        mEndDate = workdate(boost::gregorian::date(mStartDate.getGregorian().year(),12,1)); // dec same year. <shrug>
    else    
        mEndDate = workdate(simpledate(getSettingS("enddate")));

    ASSERT(!simpledate::isWeekend(mStartDate));
    ASSERT(!simpledate::isWeekend(mEndDate));

    mPort = getSettingI("port");

    if (getInputVersion()<getRequiredInputVersion())
        { TERMINATE("The input files being used require a newer version of jpf."); }
    else if (getInputVersion() > getRequiredInputVersion())
        TERMINATE(S()<<"The input files need to be updated to support the current version of jpf."
        <<"\nUpdate inputversion in settings.csv to "<<getInputVersion()<<" when done.");
}

void settings::save_settings_CSV(std::ostream & os) const
{
    os << "Setting Name,Value,Description" << std::endl;
    for (const auto & i : mValidSettings)
        {
            std::vector<std::string> vs = { i , getSettingS(i), getDescription(i) };
            simplecsv::output(os,vs);
            os << std::endl;
        }
}

std::string settings::getDescription(std::string set) const
{
    if (iSame(set,"startdate")) return "Start of scheduled period";
    if (iSame(set,"enddate")) return "Last month to display in monthly graphs";
    if (iSame(set,"inputversion")) return "Input file format version required";
    if (iSame(set,"costperdevday")) return "Cost per developer per day (including overheads)";
    if (iSame(set,"title")) return "Title for reports";
    if (iSame(set,"port")) return "Port to use when webserver run";

    TERMINATE(S()<<"Unknown setting "<<set);
    return "";
}


std::string settings::getTitle() const
{
    std::string title = getSettingS("title");
    if (title.length()==0)
        title="John's Project Forecaster";

    return title;
}


std::string settings::getSettingS(std::string settingName) const
{
    ASSERT(mLoaded);
    auto pos = mSettings.find(settingName);
    if (pos == mSettings.end()) {
        TERMINATE(S() << "Setting \"" << settingName << "\" is not defined in settings.csv.");
    }  
    return pos->second;
}

int settings::getSettingI(std::string settingName) const
{
    ASSERT(mLoaded);
    return atoi(getSettingS(settingName).c_str());
}

double settings::getSettingD(std::string settingName) const
{
    ASSERT(mLoaded);
    return strtod(getSettingS(settingName).c_str(),NULL);
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
    return monthIndex(endDate());
}

double settings::dailyDevCost() const
{
    ASSERT(mLoaded);
    return getSettingD("costperdevday");
}

settings & gSettings() {
    static settings sSettings;
    return sSettings;
}

// static
int settings::getInputVersion()
{
    int v = __INPUT_VERSION ;
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
    return getSettingI("inputversion");
}

std::string settings::getJPFVersionStr() 
{
    return __JPF_VERSION ;
}

std::string settings::getJPFReleaseStr() 
{
    return __JPF_RELEASE ;
}

void settings::setRoot(std::string path)
{
    if (path.length()==0)
        TERMINATE("Empty path when trying to set root directory.");
    
    if (path[0]=='/')
    { // absolute!
        mRootDir = std::filesystem::canonical(path);
    }
    else
    {
        char result[PATH_MAX] = {};
        getcwd(result,PATH_MAX);
        mRootDir = std::filesystem::canonical(std::string(result) + "/" + path); 
    }
}

std::string settings::getRoot() const
{
    ASSERT(mRootDir.length()>0);
    return mRootDir;
}

bool settings::RootExists() const
{
    ASSERT(mRootDir.length()>0);
    DIR* dir = opendir(getRoot().c_str());
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        return true;
    } else if (ENOENT == errno) {
        /* Directory does not exist. */
        return false;
    } else {
        /* opendir() failed for some other reason. */
        return false;
    }
}

bool settings::isValid(std::string key) const
{
    for (auto &i : mValidSettings)
        if (iSame(key,i)) return true;
    return false;
}

int settings::getPort() const
{
    return mPort;
}

void settings::advance(workdate newStart)
{
    mStartDate = newStart.getGregorian();
    mSettings["startdate"]=mStartDate.getStr();

    if (mEndDate<=mStartDate)
    {
        mEndDate = boost::gregorian::date(mStartDate.getGregorian().year()+1,1,1);
        mSettings["enddate"]=mEndDate.getStr();
    }
}

const std::string getInputPath() { return gSettings().getRoot()+"/input/"; }
const std::string getOutputPath_Txt() { return gSettings().getRoot()+"/output/"; }
const std::string getOutputPath_Html() { return gSettings().getRoot()+"/output/html/"; }
const std::string getOutputPath_Csv() { return gSettings().getRoot()+"/output/csv/"; }

