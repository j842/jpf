#ifndef __BACKLOGITEM_H
#define __BACKLOGITEM_H

#include <vector>
#include <string>
#include <ostream>

#include "workdate.h"

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
        void addToTags(std::vector<std::string> & tags) const;

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
        std::vector<std::string> mDependencies;
        std::vector<std::string> mTags;
        std::string mComments;

    private:
        void _set(const std::vector<std::string> csvitems, const unsigned int teamndx);
    };

    class teambacklogs
    {
    public:
        teambacklogs(const teams & tms);
        teambacklogs(const teambacklogs & other);
        void save_team_CSV(std::ostream &os, unsigned int teamNdx) const; // output the backlog as a inputtable csv.
        bool exists(std::string id) const;
        const backlogitem & getItemFromId(std::string id) const;

        unsigned int getTotalNumItems() const;

        std::vector<std::deque<backlogitem>> mTeamItems; // first index is team, second is backlog of items for that team.

    private:
        unsigned int mTotalNumItems;
    };

} // namespace

#endif
