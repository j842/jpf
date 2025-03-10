#ifndef __TEAMS_H
#define __TEAMS_H

#include <string>
#include <vector>

#include "utils.h"
#include "workdate.h"

namespace inputfiles
{

class teammember
{
    public:
        teammember(std::string n, tCentiDay project, tCentiDay bau, tCentiDay overhead,const std::vector<leaverange> &  l, const std::string c);
            
        const std::string mName;
        const tCentiDay mEFTProject, mEFTBAU, mEFTOverhead;

        const std::vector<leaverange> & getLeave() const;
        void advance(workdate newStart);

    private:
        std::vector<leaverange> mLeave;

    public:
        const std::string mComments;
};

class team
{
    public: 
        team(std::string Id, std::string refCode="") : mId(Id), mRefCode(refCode) {}
        std::string mId;
        std::string mRefCode;
        std::vector<teammember> mMembers;
};

class teams : public std::vector<team>
{
    public:
        teams();
        void save_teams_CSV(std::ostream & os) const;

        void debug_displayTeams() const;
        unsigned int getMaxTeamNameWidth() const;
        void advance(workdate newStart);

        const unsigned int eNotFound;
    private:
        void load_teams();

        unsigned int get_index_by_name(std::string n);
        unsigned int mMaxNameWidth;
};


}

#endif