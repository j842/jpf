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
        teammember(std::wstring n, tCentiDay project, tCentiDay bau, tCentiDay overhead, std::wstring l) : 
            mName(n), 
            mEFTProject(project), 
            mEFTBAU(bau),
            mEFTOverhead(overhead),
            mLeave(l)
         {
         }
            
        const std::wstring mName;
        const tCentiDay mEFTProject, mEFTBAU, mEFTOverhead;

        const std::wstring getLeave() const;
        void advance(workdate newStart);

    private:
        std::wstring mLeave;
};

class team
{
    public: 
        team(std::wstring Id, std::wstring refCode=L"") : mId(Id), mRefCode(refCode) {}
        std::wstring mId;
        std::wstring mRefCode;
        std::vector<teammember> mMembers;
};

class teams : public std::vector<team>
{
    public:
        teams();
        void save_teams_CSV(std::wostream & os) const;

        void debug_displayTeams() const;
        unsigned int getMaxTeamNameWidth() const;
        void advance(workdate newStart);

        const unsigned int eNotFound;
    private:
        void load_teams();

        unsigned int get_index_by_name(std::wstring n);
        unsigned int mMaxNameWidth;
};


}

#endif