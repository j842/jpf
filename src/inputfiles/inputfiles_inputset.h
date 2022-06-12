#ifndef __INPUTSET__H
#define __INPUTSET__H

#include "inputfiles_projects.h"
#include "inputfiles_publicholidays.h"
#include "inputfiles_teams.h"
#include "inputfiles_teambacklogs.h"

namespace inputfiles
{
    class teams;
    class projects;
    class teambacklogs;
    class publicholidays;

    class inputset
    {
        public:
            inputset();
            void replaceInputFiles();

        public:
            projects mProjects;
            teams mTeams;
            publicholidays mHolidays;
            teambacklogs mTeamBacklogs;
    };

}    



#endif