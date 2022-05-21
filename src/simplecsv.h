#ifndef __SIMPLECSV_H
#define __SIMPLECSV_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <ostream>
#include <cppunit/extensions/HelperMacros.h>

class simplecsv_test : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( simplecsv_test );
    CPPUNIT_TEST( splitcsv_test0 );
    CPPUNIT_TEST( splitcsv_test1 );
    CPPUNIT_TEST( splitcsv_test2 );
    CPPUNIT_TEST( splitcsv_test3 );
    CPPUNIT_TEST( splitcsv_test4 );
    CPPUNIT_TEST( roundtrip_test1 );
    CPPUNIT_TEST_SUITE_END();

    public:
        void splitcsv_test0();
        void splitcsv_test1();
        void splitcsv_test2();
        void splitcsv_test3();
        void splitcsv_test4();
        void roundtrip_test1();
};


class simplecsv 
{
    public:
        simplecsv(std::string filename, bool hasHeaderRow=true, unsigned int requiredcols=0);
        ~simplecsv();
        bool getline(std::vector<std::wstring> & line, unsigned int requiredcols);
        bool openedOkay() {return mOpenedOkay;}

        static bool splitcsv(const std::wstring s, std::vector<std::wstring> & items);

        static void output(std::wostream & os, const std::vector<std::wstring> csvitems);
        static void outputr(std::wostream & os, const std::vector<std::wstring> csvitems);
        static std::wstring makesafe(std::wstring s);
        static std::wstring trimCSVentry(const std::wstring str);
        static std::string filename2path(const std::string filename);

    private:
        std::string mFileName;
        std::wifstream mFile;
        bool mOpenedOkay;

        std::vector<std::wstring> mHeaders;
        unsigned int mHeaderCols;
};

#endif
