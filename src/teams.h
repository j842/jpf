#ifndef __TEAMS_H
#define __TEAMS_H

#include <string>
#include <vector>

#include "utils.h"

class member
{
    public:
        member(std::string n, double project, double bau, double overhead, std::string l, std::string origl) : 
            mName(n), 
            mEFTProject(project), 
            mEFTBAU(bau),
            mEFTOverhead(overhead),
            mLeave(l),
            mOriginalLeave(origl)
         {
         }
            
        const std::string mName;
        const tCentiDay mEFTProject, mEFTBAU, mEFTOverhead;
        const std::string mLeave, mOriginalLeave;
};

class team
{
    public: 
        team(std::string id) : mId(id) {}
        std::string mId;
        std::vector<member> mMembers;
};

class teams : public std::vector<team>
{
    public:
        teams();
        void load_teams();
        void save_teams_CSV(std::ostream & os) const;

        void debug_displayTeams() const;

        const unsigned int eNotFound;

        void load_public_holidays();
        void save_public_holidays_CSV(std::ostream & os) const;


    private:
        unsigned int get_index_by_name(std::string n);

        std::string mPublicHolidaysString;
};

#endif