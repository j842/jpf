#include <iostream>
#include <iomanip>
#include <climits>

#include "simplecsv.h"
#include "inputfiles_teams.h"
#include "utils.h"

namespace inputfiles
{

const std::string teammember::getLeave() const
{
    return mLeave;
}


teams::teams() : eNotFound(UINT_MAX), mMaxNameWidth(0)
{
    load_teams();
}

void teams::load_teams()
{
    simplecsv c("teams.csv");

    if (!c.openedOkay())
        TERMINATE("Could not open teams.csv!");

    mMaxNameWidth = 0;
    std::vector<std::string> row;
    while (c.getline(row, 7))
        if (row[0].length() > 0)
        {
            unsigned int ndx = get_index_by_name(row[0]);
            if (ndx == eNotFound)
            {
                this->push_back(team(row[0], row[1]));
                ndx = get_index_by_name(row[0]);
            }
            else if (!iSame(row[1], this->at(ndx).mRefCode))
                TERMINATE(S() << "Inconsistent reference codes for team " << row[0] << " : " << row[1] << " and " << this->at(ndx).mRefCode);

            mMaxNameWidth = std::max((unsigned int)row[0].length(), mMaxNameWidth);

            std::string personname = row[2];
            tCentiDay EFTProject = str2uint(row[3]);
            tCentiDay EFTBAU = str2uint(row[4]);
            tCentiDay EFTOverhead = str2uint(row[5]);

            if (EFTProject < 0 || EFTProject > 100)
                TERMINATE(S() << "EFTProject set to " << EFTProject << " for " + row[0]);
            if (EFTBAU < 0 || EFTBAU > 100)
                TERMINATE(S() << "EFTBAU set to " << EFTBAU << " for " + row[0]);
            if (EFTOverhead < 0 || EFTOverhead > 100)
                TERMINATE(S() << "EFTOverhead set to " << EFTOverhead << " for " + row[0]);
            if (EFTProject + EFTBAU + EFTOverhead > 100)
                TERMINATE(S() << personname << " is assigned to work over 100\% of the time!");

            std::string leave = row[6];
            removewhitespace(leave);

            this->at(ndx).mMembers.push_back(teammember(personname, EFTProject, EFTBAU, EFTOverhead, leave));
        }
}

void teams::save_teams_CSV(std::ostream &os) const
{
    os << R"-(Team,Ref,Person,"%EFT Projects","%EFT Other BAU","%EFT Overhead for Projects (mgmt, test)","Upcoming Leave (first + last day on leave)")-" << std::endl;

    for (unsigned int teamNdx=0 ; teamNdx < this->size() ; ++teamNdx)
    {
        auto &t = this->at(teamNdx);
        for (auto &m : this->at(teamNdx).mMembers)
        {
            std::vector<std::string> row = {t.mId, t.mRefCode, m.mName, S() << m.mEFTProject, S() << m.mEFTBAU, S() << m.mEFTOverhead, m.getLeave()}; // don't include holidays.
            simplecsv::output(os, row);
            os << std::endl;
        }
    }
}

unsigned int teams::get_index_by_name(std::string n)
{
    for (unsigned int i = 0; i < this->size(); ++i)
        if (iSame(this->at(i).mId, n))
            return i;
    return eNotFound;
}

void teams::debug_displayTeams() const
{
    std::cout << std::endl;
    std::cout << "Loaded " << this->size() << " teams:" << std::endl;
    for (auto &c : *this)
    {
        std::cout << "  " << std::setw(15) << c.mId << " -- members: ";
        for (auto &c2 : c.mMembers)
            std::cout << " { " << c2.mName << ", " << c2.mEFTProject << " } ";
        std::cout << std::endl;
    }
}

unsigned int teams::getMaxTeamNameWidth() const
{
    return mMaxNameWidth;
}

void teammember::advance(itemdate newStart)
{
    advanceLeaveString(mLeave,newStart);
}

void teams::advance(itemdate newStart)
{
    for (auto &m : *this)
        for (auto &j : m.mMembers)
            j.advance(newStart);
}

} // namespace