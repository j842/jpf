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
        resource(std::wstring n, bool b) : mName(n), mBlocking(b) {}

        std::wstring mName;
        bool mBlocking;
    };

    // task - an item in the backlog.
    class backlogitem
    {
    public:
        backlogitem(const std::wstring s, const unsigned int teamndx);
        backlogitem(const std::vector<std::wstring> csvitems, const unsigned int teamndx);
        void output(std::wostream &os) const;
        void writeresourcenames(std::wostream & oss) const; // helps with html output.

        bool hasDependency(std::wstring d);
        unsigned long getDurationDays() const;
        std::wstring getFullName() const;

    public:
        // explicitly set from CSV file.
        std::wstring mTeam;
        unsigned int mTeamNdx;
        std::wstring mId; // can be an empty string.
        std::wstring mDescription;
        std::wstring mProjectName;
        unsigned long mMinCalendarDays;
        tCentiDay mDevCentiDays;
        workdate mEarliestStart;
        std::vector<resource> mResources;
        std::vector<std::wstring> mDependencies;
        std::wstring mComments;

    private:
        void _set(const std::vector<std::wstring> csvitems, const unsigned int teamndx);
    };

    class teambacklogs
    {
    public:
        teambacklogs(const teams & tms);
        teambacklogs(const teambacklogs & other);
        void save_team_CSV(std::wostream &os, unsigned int teamNdx) const; // output the backlog as a inputtable csv.
        bool exists(std::wstring id) const;
        const backlogitem & getItemFromId(std::wstring id) const;

        unsigned int getTotalNumItems() const;

        std::vector<std::deque<backlogitem>> mTeamItems; // first index is team, second is backlog of items for that team.

    private:
        unsigned int mTotalNumItems;
    };

} // namespace

#endif
