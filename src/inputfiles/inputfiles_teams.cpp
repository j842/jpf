#include <iostream>
#include <iomanip>
#include <climits>

#include "simplecsv.h"
#include "inputfiles_teams.h"
#include "utils.h"
#include "globallogger.h"

namespace inputfiles
{

const std::string teammember::getLeave() const
{
    return mLeave;
}

void teammember::advance(workdate newStart)
{
    advanceLeaveString(mLeave,newStart);
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
    while (c.getline(row, 6))
        if (row[0].length() > 0)
        {
            unsigned int ndx = get_index_by_name(row[0]);
            if (ndx == eNotFound)
            {
                this->push_back(team(row[0]));
                ndx = get_index_by_name(row[0]);
            }
            // else if (!iSame(row[1], this->at(ndx).mRefCode))
            //     TERMINATE(S() << "Inconsistent reference codes for team " << row[0] << " : " << row[1] << " and " << this->at(ndx).mRefCode);

            mMaxNameWidth = std::max((unsigned int)row[0].length(), mMaxNameWidth);

            std::string personname = row[1];
            tCentiDay EFTProject = str2L(row[2]);
            tCentiDay EFTBAU = str2L(row[3]);
            tCentiDay EFTOverhead = str2L(row[4]);

            if (EFTProject < 0 || EFTProject > 100)
                TERMINATE(S() << "EFTProject set to " << EFTProject << " for " + row[0]);
            if (EFTBAU < 0 || EFTBAU > 100)
                TERMINATE(S() << "EFTBAU set to " << EFTBAU << " for " + row[0]);
            if (EFTOverhead < 0 || EFTOverhead > 100)
                TERMINATE(S() << "EFTOverhead set to " << EFTOverhead << " for " + row[0]);
            if (EFTProject + EFTBAU + EFTOverhead > 100)
                TERMINATE(S() << personname << " is assigned to work over 100\% of the time!");

            std::string leave = row[5];
            removewhitespace(leave);

            this->at(ndx).mMembers.push_back(teammember(personname, EFTProject, EFTBAU, EFTOverhead, leave));
        }

    // auto generate reference codes for the teams...
    unsigned int n=0;
    bool clean=false;

    while (!clean)
    {
        std::map<std::string,unsigned int> RefToTeam;
        ++n;
        clean = true;
        for (unsigned int i=0;i<this->size();++i)
            {
                std::string &inp = this->at(i).mId;
                std::string s = inp.substr(0,n);
                if (RefToTeam[s]>0)
                {
                    clean=false;
                    break;
                }
                RefToTeam[s]=i+1;
                this->at(i).mRefCode = s;
            }
        if (n>5) TERMINATE("Couldn't auto-generate team reference codes - the team names are too similar.");
    }
}

void teams::save_teams_CSV(std::ostream &os) const
{
    os << R"-(Team,Person,"%EFT Projects","%EFT Other BAU","%EFT Overhead for Projects (mgmt, test)","Upcoming Leave (first + last day on leave)")-" << std::endl;

    for (unsigned int teamNdx=0 ; teamNdx < this->size() ; ++teamNdx)
    {
        auto &t = this->at(teamNdx);
        for (auto &m : this->at(teamNdx).mMembers)
        {
            std::vector<std::string> row = {t.mId, m.mName, S() << m.mEFTProject, S() << m.mEFTBAU, S() << m.mEFTOverhead, m.getLeave()}; // don't include holidays.
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
    loginfo(S()<< "Loaded " << this->size() << " teams:" );
    for (auto &c : *this)
    {
        loginfo( S() << "  " << std::setw(15) << c.mId << " -- members: ");
        for (auto &c2 : c.mMembers)
            loginfo(S() << " { " << c2.mName << ", " << c2.mEFTProject << " } ");
    }
}

unsigned int teams::getMaxTeamNameWidth() const
{
    return mMaxNameWidth;
}

void teams::advance(workdate newStart)
{
    for (auto &m : *this)
        for (auto &j : m.mMembers)
            j.advance(newStart);
}

} // namespace