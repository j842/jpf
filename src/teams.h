#ifndef __TEAMS_H
#define __TEAMS_H

#include <string>
#include <vector>

#include "utils.h"
#include "itemdate.h"

class teammember
{
    public:
        teammember(std::string n, double project, double bau, double overhead, std::string l) : 
            mName(n), 
            mEFTProject(project), 
            mEFTBAU(bau),
            mEFTOverhead(overhead),
            mLeave(l)
         {
         }
            
        const std::string mName;
        const tCentiDay mEFTProject, mEFTBAU, mEFTOverhead;

        const std::string getLeave() const;
        void advance(itemdate newStart);

    private:
        std::string mLeave;
};

class team
{
    public: 
        team(std::string Id, std::string refCode) : mId(Id), mRefCode(refCode) {}
        std::string mId;
        std::string mRefCode;
        std::vector<teammember> mMembers;
};

class teams : public std::vector<team>
{
    public:
        teams();
        void load_teams();
        void save_teams_CSV(std::ostream & os) const;

        void debug_displayTeams() const;

        const unsigned int eNotFound;

        unsigned int getMaxTeamNameWidth() const;

        void advance(itemdate newStart);

    private:
        unsigned int get_index_by_name(std::string n);
        unsigned int mMaxNameWidth;
};

#endif