#include <iostream>
#include <iomanip>
#include <climits>

#include "simplecsv.h"
#include "teams.h"
#include "utils.h"

teams::teams() : eNotFound(UINT_MAX)
{
    load_public_holidays();

    simplecsv c("teams.csv");

    if (!c.openedOkay())
        TERMINATE("Could not open teams.csv!");

    std::vector<std::string> row;
    while (c.getline(row,6))
    if (row[0].length()>0)
        {
            unsigned int ndx=get_index_by_name(row[0]);
            if (ndx==eNotFound)
            {
                this->push_back(team(row[0]));
                ndx=get_index_by_name(row[0]);
            }

            std::string name = row[1];
            tCentiDay EFTProject  = str2uint(row[2]);
            tCentiDay EFTBAU      = str2uint(row[3]);
            tCentiDay EFTOverhead = str2uint(row[4]);

            if (EFTProject<0 || EFTProject>100)
                TERMINATE(S()<<"EFTProject set to "<<EFTProject<<" for "+row[0]);
            if (EFTBAU<0 || EFTBAU>100)
                TERMINATE(S()<<"EFTBAU set to "<<EFTBAU<<" for "+row[0]);
            if (EFTOverhead<0 || EFTOverhead>100)
                TERMINATE(S()<<"EFTOverhead set to "<<EFTOverhead<<" for "+row[0]);
            if (EFTProject + EFTBAU + EFTOverhead > 100 )
                TERMINATE(S()<<row[1]<<" is assigned to work over 100\% of the time!");


            std::string leave = row[5];
            removewhitespace(leave);
            if (leave.length()>0) leave+=",";
            leave += mPublicHolidaysString;

            this->at(ndx).mMembers.push_back( member(name,EFTProject,EFTBAU,EFTOverhead,leave));
        }
}

unsigned int teams::get_index_by_name(std::string n)
{
    for (unsigned int i=0;i<this->size();++i)
        if (iSame(this->at(i).mId,n)) return i;
    return eNotFound;
}


void teams::debug_displayTeams() const
{
    std::cout << std::endl;
    std::cout << "Loaded " << this->size() << " teams:" << std::endl;
    for (auto & c : *this)
    {
        std::cout << "  " << std::setw(15) << c.mId << " -- members: ";
        for (auto & c2 : c.mMembers)
            std::cout << " { " << c2.mName << ", "<<c2.mEFTProject<<" } ";
        std::cout<<std::endl;
    }
}


void teams::load_public_holidays()
{
    simplecsv ph("publicholidays.csv");

    if (!ph.openedOkay())
    {
        std::cerr << "Could not read the public holidays from publicholidays.csv"<<std::endl;
        return;
    }

    std::vector<std::string> row;
    while (ph.getline(row,1))
    {
        if (mPublicHolidaysString.length()>0)
            mPublicHolidaysString += ",";
        mPublicHolidaysString += row[0];
    }

//    std::cout << "Upcoming public holidays: "<< mPublicHolidaysString << std::endl;
}

void teams::save_public_holidays_CSV(std::ostream & os) const
{
    os << "Public Holdiay (Date or Range)" << std::endl;

    std::vector<std::string> vd;
    simplecsv::splitcsv(mPublicHolidaysString,vd);
    for (auto d : vd)
        os << simplecsv::makesafe(d) << std::endl;
}

