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
        void save_settings_CSV(std::wostream & os) const;
        bool loaded() const {return mLoaded;}

        void advance(workdate newStart);

        simpledate startDate() const;
        simpledate endDate() const;
        monthIndex endMonth() const;
        double dailyDevCost() const;

        int getRequiredInputVersion() const;

        eLogLevel getMinLogLevel() const;

        std::wstring getRoot() const;
        bool RootExists() const;

        static int getInputVersion();
        static std::wstring getInputVersionStr();
        static std::wstring getJPFVersionStr();
        static std::wstring getJPFReleaseStr();
        static std::wstring getJPFFullVersionStr();

        std::wstring getTitle() const;
        int getPort() const;

    public:
        void setSettings( simpledate startDate, simpledate endDate, int port  ); // for testing.
        void setMinLogLevel(eLogLevel l);

    private:
        std::wstring getDescription(std::wstring set) const;

        std::wstring getSettingS(std::wstring settingName, const std::map<std::wstring,std::wstring> & settings) const;
        int getSettingI(std::wstring settingName, const std::map<std::wstring,std::wstring> & settings) const;     
        double getSettingD(std::wstring settingName,  const std::map<std::wstring,std::wstring> & settings) const;     
        bool isValid(std::wstring key) const;

    private:
        bool mLoaded;
        std::string mRootDir;
        simpledate mStartDate;
        simpledate mEndDate;
        int mPort;
        eLogLevel mMinLogLevel;
        int mRequiredInputVersion;
        double mDailyDevCost;
        std::wstring mTitle;

        std::vector<std::wstring> mValidSettings;
};

settings & gSettings();

const std::string getInputPath();
const std::string getOutputPath_Base();
const std::string getOutputPath_Txt();
const std::string getOutputPath_Html();
const std::string getOutputPath_Csv();
const std::string getOutputPath_Log();
const std::string getOptTemplatesPath();

#endif
