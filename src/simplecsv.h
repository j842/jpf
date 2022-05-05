#ifndef __SIMPLECSV_H
#define __SIMPLECSV_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

class simplecsv_test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( simplecsv_test );
    CPPUNIT_TEST( splitcsv_test0 );
    CPPUNIT_TEST( splitcsv_test1 );
    CPPUNIT_TEST( splitcsv_test2 );
    CPPUNIT_TEST_SUITE_END();

    public:
        void splitcsv_test0();
        void splitcsv_test1();
        void splitcsv_test2();
};


class simplecsv 
{
    public:
        simplecsv(std::string filename, bool hasHeaderRow=true, unsigned int requiredcols=0);
        ~simplecsv();
        bool getline(std::vector<std::string> & line, unsigned int requiredcols);
        bool openedOkay() {return mOpenedOkay;}

        static bool splitcsv(const std::string s, std::vector<std::string> & items);

    private:
        std::string mFileName;
        std::ifstream mFile;
        bool mOpenedOkay;

        std::vector<std::string> mHeaders;
        unsigned int mHeaderCols;
};

#endif
