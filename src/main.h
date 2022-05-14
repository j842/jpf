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
        int showhelp();
        int create_directories();
        int advance(std::string s);

        int getrVal() const;

    private:
        int go(int argc, char **argv);
        void replace_all_input_CSV_files(inputfiles::inputset iset);

    private:
        int mrVal;
};

int main(int argc, char **argv);


#endif