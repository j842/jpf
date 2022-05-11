#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <string>
#include <map>
#include <boost/date_time.hpp>

#include "itemdate.h"

class settings
{
    public:
        settings();
        void setRoot(std::string path);
        void load_settings();
        void save_settings_CSV(std::ostream & os) const;

        void advance(itemdate newStart);

        boost::gregorian::date startDate() const;
        boost::gregorian::date endDate() const;
        double dailyDevCost() const;

        int getRequiredInputVersion() const;

        std::string getRoot() const;
        bool RootExists() const;

        static int getInputVersion();
        static std::string getInputVersionStr();
        static std::string getJPFVersionStr();
        static std::string getJPFReleaseStr();

        std::string getTitle() const;
        int getPort() const;

    private:
        std::string getSettingS(std::string settingName) const;
        int getSettingI(std::string settingName) const;     
        double getSettingD(std::string settingName) const;     
        bool isValid(std::string key) const;
        std::string getDescription(std::string set) const;

        bool mLoaded;
        boost::gregorian::date mStartDate;
        boost::gregorian::date mEndDate;
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
