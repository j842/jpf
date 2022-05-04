#include <sstream>

#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

simplecsv::simplecsv(std::string filename, bool hasHeaderRow, unsigned int requiredcols) :  
    mFileName(filename), mFile(getInputPath()+filename), mOpenedOkay(false)
{
    mOpenedOkay = mFile.is_open();
    
    
    if (hasHeaderRow && mOpenedOkay) 
        mOpenedOkay = getline(mHeaders,mHeaderCols,requiredcols);
}

simplecsv::~simplecsv()
{
}

bool simplecsv::splitcsv(const std::string s,
    std::vector<std::string> & items, unsigned int & numitems)
    {
    bool inquotes=false;
    std::string item;

    if (s.length()==0 || s[0]=='\r' || s[0]=='\n')
        return false;

    items.clear();
    items.push_back("");
    numitems=1;

    for (unsigned int i=0;i<s.length();++i)
    {
        switch (s[i])
        {
            case '\\':
                ++i; // skip escaped characters, except quotes.
                if (i<s.length() && s[i]=='"')
                    items[numitems-1].push_back('"');
                break;
            case '"':
                inquotes=!inquotes;
            case '\r':
                break;
            case ',':
                if (inquotes)
                    items[numitems-1].push_back(',');
                else
                {
                    ++numitems;
                    items.push_back("");
                }
                break;
            default:
                items[numitems-1].push_back(s[i]); // slow but meh.
        };
    }

    if (inquotes)
        {
            std::cerr << "Unclosed quotes encountered in CSV string:" 
                << std::endl << s << std::endl;
            return false;
        }

    return true;
    }

bool simplecsv::getline(std::vector<std::string> & line, unsigned int & numcols, unsigned int requiredcols)
{
    line.clear();
    numcols=0;

    if (!mFile.is_open())
        return false;

    std::string row;
    while (row.length()==0) // skip empty rows.
    {
        if (!std::getline(mFile, row))
            return false;

        // skip rows with incorrect syntax (e.g. blank lines)
        if (!splitcsv(row,line,numcols))
            row.clear();

        // skip rows that have correct syntax, but have no data.
        bool allempty=true;
        for (unsigned int i=0;i<numcols;++i)
            if (line[i].length()>0)
                allempty=false;
        if (allempty)
            row.clear();
    }

    if (requiredcols>0 && numcols != requiredcols)
    {
        TERMINATE( S() 
            << "While parsing CSV row, required "<<requiredcols
            << " columns but received "<<numcols<<std::endl << std::endl
            << "The offending line was:" << std::endl
            << row << std::endl );
    }

    return true;
}

