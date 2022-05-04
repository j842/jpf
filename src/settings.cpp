#include <climits>
#include <strings.h>
#include <dirent.h>
#include <errno.h>
#include <filesystem>

#include "settings.h"
#include "simplecsv.h"
#include "utils.h"
#include "itemdate.h"
#include "version.h"

settings::settings() : mLoaded(false), mRootDir()
{
}

void settings::load_settings()
{
    simplecsv c("settings.csv");

    if (!c.openedOkay())
        TERMINATE("Could not open "+getInputPath()+"settings.csv.");

    std::vector<std::string> row;
    unsigned int n;
    while (c.getline(row,n,2))
        mSettings[row[0]]=row[1];

    mLoaded=true;

    std::string sdate=getSettingS("startdate");
    if (sdate.length()==0 || iSame(sdate,"today"))
        mStartDate = itemdate::nextWorkday(boost::gregorian::day_clock::local_day());
    else
        mStartDate = itemdate::nextWorkday(itemdate::parseDateStringDDMMYY(getSettingS("startdate")));

    std::string edate=getSettingS("enddate");
    if (sdate.length()==0 || iSame(sdate,"forever"))
        mEndDate = itemdate::nextWorkday(boost::gregorian::date(mStartDate.year(),11,1));
    else    
        mEndDate = itemdate::parseDateStringDDMMYY(getSettingS("enddate"));

    if (getInputVersion()<getRequiredInputVersion())
        { TERMINATE("The input files being used require a newer version of jpf."); }
    else if (getInputVersion() > getRequiredInputVersion())
        TERMINATE(S()<<"The input files need to be updated to support the current version of jpf."
        <<"\nUpdate inputversion in settings.csv to "<<getInputVersion()<<" when done.");
}

std::string settings::getSettingS(std::string settingName) const
{
    ASSERT(mLoaded);
    auto pos = mSettings.find(settingName);
    if (pos == mSettings.end()) {
        TERMINATE(S() << "Setting " << settingName << " is not defined in settings.csv");
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


boost::gregorian::date settings::startDate() const 
{
    ASSERT(mLoaded);
    return mStartDate;
}

boost::gregorian::date settings::endDate() const
{
    ASSERT(mLoaded);
    return mEndDate;
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

const std::string getInputPath() { return gSettings().getRoot()+"/input/"; }
const std::string getOutputPath_Txt() { return gSettings().getRoot()+"/output/"; }
const std::string getOutputPath_Html() { return gSettings().getRoot()+"/output/html/"; }
const std::string getOutputPath_Csv() { return gSettings().getRoot()+"/output/csv/"; }

