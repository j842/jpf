#ifndef __BACKLOGITEM_H
#define __BACKLOGITEM_H

#include <vector>
#include <string>
#include <ostream>

#include "workdate.h"
#include "ctags.h"

namespace inputfiles
{
    class projects; // forward decl
    class teams;

    // people resource.
    class resource
    {
    public:
        resource() : mBlocking(false) {}
        resource(const resource &o) : mName(o.mName), mBlocking(o.mBlocking) {}
        resource(std::string n, bool b) : mName(n), mBlocking(b) {}
        
        operator std::string() const {return mName;}

        std::string mName;
        bool mBlocking;
    };

    // task - an item in the backlog.
    class backlogitem
    {
    public:
        backlogitem(const std::string s, const unsigned int teamndx);
        backlogitem(const std::vector<std::string> csvitems, const unsigned int teamndx);
        void output(std::ostream &os) const;
        void writeresourcenames(std::ostream & oss) const; // helps with html output.

        bool hasDependency(std::string d);
        unsigned long getDurationDays() const;
        std::string getFullName() const;

        bool hasTag(std::string tag) const;
        void copyinto(std::vector<std::string> & tags) const;

    public:
        // explicitly set from CSV file.
        std::string mTeam;
        unsigned int mTeamNdx;
        std::string mId; // can be an empty string.
        std::string mDescription;
        std::string mProjectName;
        unsigned long mMinCalendarDays;
        tCentiDay mDevCentiDays;
        workdate mEarliestStart;
        std::vector<resource> mResources;
        cTags mDependencies; // can be task Ids or Project Ids
        cTags mTags;
        std::string mComments;

    private:
        void _set(const std::vector<std::string> csvitems, const unsigned int teamndx);
    };

    typedef enum 
    {
        kInvalid = 0,
        kProject,
        kTask
    } tDepType;

    class teambacklogs;

    class depRef
    {
        public:
            depRef(std::string dep, const teambacklogs & tbl, const projects &pjts);

            tDepType getType() const;

            unsigned int getProjectNdx() const;
            void getTeamItemNdx(unsigned int & teamndx, unsigned int & itemindexinteam) const;
            std::string getRefString() const;

        private:
            std::string mDep;
            tDepType mType;
            unsigned int mProjectNdx;
            unsigned int mTeamNdx;
            unsigned int mItemIndexinTeam;
    };

    class teambacklogs
    {
    public:
        teambacklogs(const teams & tms,const projects &pjts);
        teambacklogs(const teambacklogs & other);
        void save_team_CSV(std::ostream &os, unsigned int teamNdx) const; // output the backlog as a inputtable csv.
        bool exists(std::string id) const;
        const backlogitem & getItemFromId(std::string id) const;

        unsigned int getTotalNumItems() const;

        std::vector<std::deque<backlogitem>> mTeamItems; // first index is team, second is backlog of items for that team.

    private:
        void validateBacklogItem(const backlogitem & b, const teams & tms,const projects &pjts, unsigned int taskNum, std::string filename, std::string row) const;

    private:
        unsigned int mTotalNumItems;
    };

} // namespace

#endif
