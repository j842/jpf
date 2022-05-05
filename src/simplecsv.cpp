#include <sstream>

#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

CPPUNIT_TEST_SUITE_REGISTRATION( simplecsv_test );


simplecsv::simplecsv(std::string filename, bool hasHeaderRow, unsigned int requiredcols) :  
    mFileName(filename), mFile(getInputPath()+filename), mOpenedOkay(false)
{
    mOpenedOkay = mFile.is_open();
    
    
    if (hasHeaderRow && mOpenedOkay) 
        mOpenedOkay = getline(mHeaders,requiredcols);
    mHeaderCols = mHeaders.size();
}

simplecsv::~simplecsv()
{
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
    CPPUNIT_ASSERT(strcmp(items[0].c_str(),"a")==0);
    CPPUNIT_ASSERT(strcmp(items[1].c_str(),"b")==0);
    CPPUNIT_ASSERT(strcmp(items[2].c_str(),"c")==0);
    CPPUNIT_ASSERT(strcmp(items[3].c_str(),"d")==0);
    CPPUNIT_ASSERT(strcmp(items[4].c_str(),"")==0);
}
void simplecsv_test::splitcsv_test0( )
{
    std::string s( R"literal("\\c,\,\,"\,\ )literal" );
    std::vector<std::string> items;

    simplecsv::splitcsv(s,items);

    CPPUNIT_ASSERT(items.size()==1);
    CPPUNIT_ASSERT_MESSAGE(S()<<items[0]<< "   is not   \\c,,,,", strcmp(items[0].c_str(),"\\c,,,,")==0);
}

bool simplecsv::splitcsv(const std::string s, // copy
    std::vector<std::string> & items)
    {
    items.clear();
    if (s.length()==0 || s[0]=='\r' || s[0]=='\n')
        return false;

    bool inquotes=false;
    unsigned int startpos=0;
    for (unsigned int i=0;i<s.length();++i)
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
                    if (i>startpos)
                        items.push_back(s.substr(startpos,i-startpos));
                    else
                        items.push_back("");
                    startpos=i+1;

                    if (startpos>=s.length()) 
                    {
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
            std::cout << "Unmatched quotes in csv row: "<<std::endl<<s<<std::endl;
            return false;
        }

        ASSERT(startpos<s.length());
        items.push_back(s.substr(startpos,s.length()-startpos));


        // now remove whitespace
        inquotes=false;
        for (auto & ii : items)
        {
            trim(ii);
            
            for (unsigned int i=0;i<ii.size();++i)
                switch (ii[i])
                {
                    case '"':
                        ii.erase(i,1);
                        --i;
                        break;
                    case '\\':
                        ii.erase(i,1);
                        break;
                    default:
                        break;
                }

        }

        return true;
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

