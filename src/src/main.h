#ifndef __MAIN_H
#define __MAIN_H

#include "args.h"
#include <map>

class cMain
{
    public:
        cMain();
        int go(cArgs args);

        bool runtests();
        int run_console();
        int run_watch();
        int run_refresh();
        int run_advance(std::string s);
        int showhelp();

    private:
       void replace_settings_CSV();
};

int main(int argc, char **argv);


#endif