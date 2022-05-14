#ifndef __BACKLOGITEM_H
#define __BACKLOGITEM_H

#include <vector>
#include <string>
#include <ostream>

#include "itemdate.h"

namespace inputfiles
{

    class projects; // forward decl
    class teams;

    // people resource.
    class resource
    {
    public:
        resource() : mBlocking(false), mLoadingPercent(0.0) {}
        resource(const resource &o) : mName(o.mName), mBlocking(o.mBlocking), mLoadingPercent(0.0) {}
        resource(std::string n, bool b) : mName(n), mBlocking(b), mLoadingPercent(0.0) {}

        std::string mName;
        bool mBlocking;
        double mLoadingPercent;
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
        itemdate getDuration() const;
        std::string getFullName() const;

    public:
        // explicitly set from CSV file.
        std::string mTeam;
        unsigned int mTeamNdx;
        std::string mId; // can be an empty string.
        std::string mDescription;
        std::string mProjectName;
        unsigned long mMinCalendarDays;
        tCentiDay mDevCentiDays;
        itemdate mEarliestStart;
        std::vector<resource> mResources;
        std::vector<std::string> mDependencies;
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

        std::vector<std::deque<backlogitem>> mTeamItems; // first index is team, second is backlog of items for that team.
    };

} // namespace

#endif
