#ifndef __MAIN_H
#define __MAIN_H

class cMain
{
    public:
        cMain(int argc, char **argv);

        bool runtests();
        int run_console();
        int run_watch();
        int run_refresh();
        int run_advance(std::wstring s);
        int run_create_directories();
        int showhelp();

        int getrVal() const;

    private:
        int go(int argc, char **argv);
        void replace_all_input_CSV_files(inputfiles::inputset iset);
        void replace_settings_CSV();
    private:
        int mrVal;
};

int main(int argc, char **argv);


#endif