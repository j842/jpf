#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <string>
#include <map>
#include <boost/date_time.hpp>

class settings
{
    public:
        settings();
        void setRoot(std::string path);
        void load_settings();

        boost::gregorian::date startDate() const;
        boost::gregorian::date endDate() const;
        double dailyDevCost() const;

        int getRequiredInputVersion() const;

        std::string getRoot() const;
        bool RootExists() const;

        static int getInputVersion();
        static std::string getInputVersionStr();
        static std::string getJPFVersionStr();

    private:
        bool mLoaded;
        boost::gregorian::date mStartDate;
        boost::gregorian::date mEndDate;
        std::string mRootDir;

        std::string getSettingS(std::string settingName) const;
        int getSettingI(std::string settingName) const;     
        std::map<std::string,std::string> mSettings;   
};

settings & gSettings();

const std::string getInputPath();
const std::string getOutputPath_Txt();
const std::string getOutputPath_Html();
const std::string getOutputPath_Csv();

#endif
