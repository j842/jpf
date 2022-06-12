#include "inputfiles_inputset.h"

namespace inputfiles
{

    inputset::inputset() :
        mProjects(),mTeams(),mHolidays(),mTeamBacklogs(mTeams,mProjects)
    {
    }



void inputset::replaceInputFiles()
{
    ASSERT(mTeams.size()==mTeamBacklogs.mTeamItems.size());
    for (unsigned int i = 0; i < mTeams.size(); ++i)
    { // output backlog-X
        std::ofstream ofs(simplecsv::filename2path(S() << "backlog-" << makelower(mTeams[i].mId) << ".csv"));
        mTeamBacklogs.save_team_CSV(ofs, i);
        ofs.close();
    }

    {
        std::ofstream ofs(simplecsv::filename2path("publicholidays.csv"));
        mHolidays.save_public_holidays_CSV(ofs);
        ofs.close();
    }

    {
        std::ofstream ofs(simplecsv::filename2path("teams.csv"));
        mTeams.save_teams_CSV(ofs);
        ofs.close();
    }

    {
        std::ofstream ofs(simplecsv::filename2path("projects.csv"));
        mProjects.save_projects_CSV(ofs);
        ofs.close();
    }
}


} // namespace