#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <string>
#include <map>
#include <boost/date_time.hpp>

#include "workdate.h"

class settings
{
    public:
        settings();
        void setRoot(std::string path);
        void load_settings();
        void save_settings_CSV(std::ostream & os) const;

        void advance(workdate newStart);

        simpledate startDate() const;
        simpledate endDate() const;
        monthIndex endMonth() const;
        double dailyDevCost() const;

        int getRequiredInputVersion() const;

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

    private:
        std::string getSettingS(std::string settingName) const;
        int getSettingI(std::string settingName) const;     
        double getSettingD(std::string settingName) const;     
        bool isValid(std::string key) const;
        std::string getDescription(std::string set) const;

        bool mLoaded;
        simpledate mStartDate;
        simpledate mEndDate;
        std::string mRootDir;
        int mPort;

        std::map<std::string,std::string> mSettings;   
        const std::vector<std::string> mValidSettings;
};

settings & gSettings();

const std::string getInputPath();
const std::string getOutputPath_Txt();
const std::string getOutputPath_Html();
const std::string getOutputPath_Csv();

#endif
