#include <sstream>

#include "simplecsv.h"
#include "../utils.h"
#include "../settings.h"

CPPUNIT_TEST_SUITE_REGISTRATION( simplecsv_test );


simplecsv::simplecsv(std::string filename, bool hasHeaderRow, unsigned int requiredcols) :  
    mFileName(filename), mFile(filename2path(filename)), mOpenedOkay(false)
{
    mOpenedOkay = mFile.is_open();
    
    
    if (hasHeaderRow && mOpenedOkay) 
        mOpenedOkay = getline(mHeaders,requiredcols);
    mHeaderCols = mHeaders.size();
}

simplecsv::~simplecsv()
{
}

std::string simplecsv::filename2path(const std::string filename)
{
    return getInputPath()+filename;
}


bool simplecsv::splitcsv(const std::string s, // copy
                         std::vector<std::string> &items)
{
    items.clear();
    if (s.length() == 0 || s[0] == '\r' || s[0] == '\n')
        return false;

    bool inquotes = false;
    unsigned int startpos = 0;
    for (unsigned int i = 0; i < s.length(); ++i)
        switch (s[i])
        {
        case '\\':
            i++; // skip processing of next character.
            break;

        case '"':
            inquotes = !inquotes;
            break;

        case ',':
            if (!inquotes)
            {
                if (i > startpos)
                    items.push_back( trimCSVentry( s.substr(startpos, i - startpos) ));
                else
                    items.push_back("");
                startpos = i + 1;

                if (startpos >= s.length())
                { // string ends on a comma
                    items.push_back("");
                    return true; // done!
                }
            }
            break;
        default:
            break;
        };

    if (inquotes)
    {
        logerror( S() << "Unmatched quotes in csv row: \n" << s);
        return false;
    }

    ASSERT(startpos < s.length());
    items.push_back( trimCSVentry( s.substr(startpos, s.length() - startpos) ) );
    return true;
}


std::string simplecsv::trimCSVentry(const std::string str)
{
    std::string s(str);

    // now remove whitespace
    trim(s);
    bool inquotes=false;

    for (unsigned int i = 0; i < s.size(); ++i)
        switch (s[i])
        {
        case '"':
            if (inquotes && i+2<s.size() && s[i+1]=='"') // double quotes, within a quoted string. Keep one as per CSV standard (MIME etc).
                break;

            s.erase(i, 1); // erase all quotes.
            --i;
            if (!inquotes)
                inquotes=true;
            break;
        case '\\':
            s.erase(i, 1);
            break;
        default:
            break;
        }

    return s;
}

bool simplecsv::getline(std::vector<std::string> & line, unsigned int requiredcols)
{
    line.clear();

    if (!mFile.is_open())
        return false;

    std::string row;
    while (row.length()==0) // skip empty rows.
    {
        if (!std::getline(mFile, row))
            return false;

        // skip rows with incorrect syntax (e.g. blank lines)
        if (!splitcsv(row,line))
            row.clear();

        // skip rows that have correct syntax, but have no data.
        bool allempty=true;
        for (unsigned int i=0;i<line.size();++i)
            if (line[i].length()>0)
                allempty=false;
        if (allempty)
            row.clear();
    }

    if (requiredcols>0 && line.size() != requiredcols)
    {
        TERMINATE( S() 
            << "While parsing CSV row, required "<<requiredcols
            << " columns but received "<<line.size()<<std::endl << std::endl
            << "The offending line was:" << std::endl
            << row << std::endl );
    }

    return true;
}

std::string simplecsv::makesafe(std::string s)
{
    std::string ss;
    bool needsquotes=false;
    for (unsigned int i=0;i<s.length();++i)
    {
        switch (s[i])
        {
            case ',':
                needsquotes=true;
                break;
            case '"':
                needsquotes=true;
                ss.push_back('"'); // quotes come out double.
                break;
            case '\\':
                ss.push_back('\\'); // slashes are escaped.
                break;
        }
        ss.push_back(s[i]);
    }
    if (needsquotes)
    {
        ss.insert(ss.begin(),'\"');
        ss.push_back('"');
    }
    return ss;
}


void simplecsv::output(std::ostream & os, const std::vector<std::string> & csvitems)
{
    for (unsigned int i=0;i<csvitems.size();++i)
        if (i>0)
            os << "," << makesafe(csvitems[i]);
        else
            os << makesafe(csvitems[i]);
}

void simplecsv::outputr(std::ostream & os, const std::vector<std::string> & csvitems)
{
    output(os,csvitems);
    os << std::endl;
}



// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------


void simplecsv_test::splitcsv_test4()
{
    std::string s( R"literal(Stu,"""W1,W3""","hello "" hello")literal" );
    std::vector<std::string> items;
    simplecsv::splitcsv(s,items);
    CPPUNIT_ASSERT(items.size()==3);
    CPPUNIT_ASSERT_MESSAGE(S() << " Different:  "<<items[1]<< "   \"W1,W3\"",iSame(items[1],"\"W1,W3\""));
    CPPUNIT_ASSERT_MESSAGE(S() << " Different:  "<<items[2]<< "   hello \" hello",iSame(items[2],"hello \" hello"));
}

void simplecsv_test::splitcsv_test3()
{
    std::string s( R"literal(Stu,"W1,W3",)literal" );
    std::vector<std::string> items;
    simplecsv::splitcsv(s,items);
    CPPUNIT_ASSERT(items.size()==3);
    CPPUNIT_ASSERT_MESSAGE(S() << " Different:  "<<items[1]<< "   W1,W3",iSame(items[1],"W1,W3"));
}
void simplecsv_test::splitcsv_test2( )
{
    std::string s( R"literal(   a  ,,b,,"\\c,,,,,","" )literal" );
    std::vector<std::string> items;

    simplecsv::splitcsv(s,items);

    CPPUNIT_ASSERT(items.size()==6);
    CPPUNIT_ASSERT(strcmp(items[0].c_str(),"a")==0);
    CPPUNIT_ASSERT(strcmp(items[1].c_str(),"")==0);
    CPPUNIT_ASSERT(strcmp(items[2].c_str(),"b")==0);
    CPPUNIT_ASSERT(strcmp(items[3].c_str(),"")==0);
    CPPUNIT_ASSERT_MESSAGE(S()<<items[4]<< "   is not   \\c,,,,,",strcmp(items[4].c_str(),"\\c,,,,,")==0);
    CPPUNIT_ASSERT_MESSAGE(S()<<">>"<<items[5]<< "<<   is not   >><<", strcmp(items[5].c_str(),"")==0);
}
void simplecsv_test::splitcsv_test1( )
{
    std::string s("a,b,c,d,");
    std::vector<std::string> items;

    simplecsv::splitcsv(s,items);

    CPPUNIT_ASSERT(items.size()==5);
    CPPUNIT_ASSERT(items[0].compare("a")==0);
    CPPUNIT_ASSERT(items[1].compare("b")==0);
    CPPUNIT_ASSERT(items[2].compare("c")==0);
    CPPUNIT_ASSERT(items[3].compare("d")==0);
    CPPUNIT_ASSERT(items[4].compare("")==0);
}
void simplecsv_test::splitcsv_test0( )
{
    std::string s( R"literal("\\c,\,\,"\,\ )literal" );
    std::vector<std::string> items;

    simplecsv::splitcsv(s,items);

    CPPUNIT_ASSERT(items.size()==1);
    CPPUNIT_ASSERT_MESSAGE(S()<<items[0]<< "   is not   \\c,,,,", strcmp(items[0].c_str(),"\\c,,,,")==0);
}

void simplecsv_test::roundtrip_test1( )
{
    std::string s( R"literal(a,12,3.721,dog dog dog,"""heroes"" never die",,,"a,""b"",c")literal" );
    std::vector<std::string> items;
    simplecsv::splitcsv(s,items);
    std::ostringstream oss;
    simplecsv::output(oss, items);
    CPPUNIT_ASSERT_MESSAGE(S()<<std::endl<<"["<<s<<"]"<<std::endl<<" is not "<<std::endl<<"["<<oss.str()<<"]"<<std::endl,
        strcmp(s.c_str(),oss.str().c_str())==0);
}