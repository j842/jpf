#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <string>
#include <map>
#include <boost/date_time.hpp>

#include "workdate.h"
#include "globallogger.h"

class settings
{
    public:
        settings();
        void setRoot(std::string path);
        void load_settings();
        void save_settings_CSV(std::ostream & os) const;
        bool loaded() const {return mLoaded;}

        void advance(workdate newStart);

        simpledate startDate() const;
        simpledate endDate() const;
        monthIndex endMonth() const;
        double dailyDevCost() const;

        int getRequiredInputVersion() const;

        eLogLevel getMinLogLevel() const;

        std::string getRoot() const;
        bool RootExists() const;

        static int getInputVersion();
        static std::string getInputVersionStr();
        static std::string getJPFVersionStr();
        static std::string getJPFReleaseStr();
        static std::string getJPFFullVersionStr();

        std::string getTitle() const;
        int getPort() const;

    public:
        void setSettings( simpledate startDate, simpledate endDate, int port  ); // for testing.
        void setMinLogLevel(eLogLevel l);

    private:
        std::string getDescription(std::string set) const;

        std::string getSettingS(std::string settingName, const std::map<std::string,std::string> & settings) const;
        int getSettingI(std::string settingName, const std::map<std::string,std::string> & settings) const;     
        double getSettingD(std::string settingName,  const std::map<std::string,std::string> & settings) const;     
        bool isValid(std::string key) const;

    private:
        bool mLoaded;
        std::string mRootDir;
        simpledate mStartDate;
        simpledate mEndDate;
        int mPort;
        eLogLevel mMinLogLevel;
        int mRequiredInputVersion;
        double mDailyDevCost;
        std::string mTitle;

        std::vector<std::string> mValidSettings;
};

settings & gSettings();

const std::string getInputPath();
const std::string getOutputPath_Base();
const std::string getOutputPath_Txt();
const std::string getOutputPath_Html();
const std::string getOutputPath_Csv();
const std::string getOutputPath_Log();

const std::string getOptHTMLPath();

#endif
