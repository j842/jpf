#ifndef __MAIN_H
#define __MAIN_H

#include "args.h"
#include <map>

class cLocalSettings
{   
    public:
        cLocalSettings();
        void setSetting(std::string key, std::string value);
        std::string getSetting(std::string key) const;
        bool isLoaded() const;

    private:
        void save() const;
        const std::string mFilePath;
        std::map<std::string,std::string> mSettings;
        bool mLoaded;
};

class cMain
{
    public:
        cMain(cArgs args);

        bool runtests();
        int run_console();
        int run_watch();
        int run_refresh();
        int run_advance(std::string s);
        int run_create_directories();
        int showhelp();

        int getrVal() const;

    private:
        int go(cArgs args);
        void replace_all_input_CSV_files(inputfiles::inputset iset);
        void replace_settings_CSV();
    private:
        int mrVal;
};

int main(int argc, char **argv);


#endif