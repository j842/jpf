#ifndef __TEAMS_H
#define __TEAMS_H

#include <string>
#include <vector>

#include "utils.h"

class member
{
    public:
        member(std::string n, double project, double bau, double overhead, std::string l) : 
            mName(n), 
            mEFTProject(project), 
            mEFTBAU(bau),
            mEFTOverhead(overhead),
            mLeave(l) {}
            
        const std::string mName;
        const tCentiDay mEFTProject, mEFTBAU, mEFTOverhead;
        const std::string mLeave;
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

        void debug_displayTeams() const;

        const unsigned int eNotFound;


    private:
        unsigned int get_index_by_name(std::string n);

};

#endif