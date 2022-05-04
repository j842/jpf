#ifndef __SIMPLECSV_H
#define __SIMPLECSV_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class simplecsv
{
    public:
        simplecsv(std::string filename, bool hasHeaderRow=true, unsigned int requiredcols=0);
        ~simplecsv();
        bool getline(std::vector<std::string> & line, unsigned int & numcols, unsigned int requiredcols);
        bool openedOkay() {return mOpenedOkay;}

        static bool splitcsv(const std::string s,
            std::vector<std::string> & items, unsigned int & numitems);


    private:
        std::string mFileName;
        std::ifstream mFile;
        bool mOpenedOkay;

        std::vector<std::string> mHeaders;
        unsigned int mHeaderCols;
};

#endif
