#ifndef __BACKLOGITEM_H
#define __BACKLOGITEM_H

#include <vector>
#include <string>
#include <ostream>

#include "projects.h"

// people resource.
class resource
{
    public:
        resource() : mBlocking(false), mLoadingPercent(0.0) {}
        resource(const resource & o) : mName(o.mName), mBlocking(o.mBlocking), mLoadingPercent(0.0) {}
        resource(std::string n, bool b) : mName(n), mBlocking(b), mLoadingPercent(0.0) {}

        std::string mName;
        bool mBlocking;
        double mLoadingPercent;
};

// task - an item in the backlog.
class backlogitem
{
    public:
        backlogitem(const std::string s, const unsigned int teamndx, const projects &p);
        backlogitem(const std::vector<std::string> csvitems, const unsigned int teamndx, const projects &p);
        void set(const std::vector<std::string> csvitems, const unsigned int teamndx, const projects &p);
        void output(std::ostream & os, const projects &p) const;

        bool hasDependency(std::string d);
        itemdate getDuration() const;
        std::string getFullName() const;

        // explicitly set from CSV file.
        unsigned int mTeamNdx;
        unsigned int mProject;
        std::string mId; // can be an empty string.
        std::string mDescription;
        std::string mProjectName;
        int mMinCalendarDays;
        int mDevDays;
        itemdate mEarliestStart;
        std::vector<resource> mResources;
        std::vector<std::string> mDependencies;
        std::string mComments;

        // set while scheduling the task.
        unsigned int mPriority;
        unsigned int mMergePriority;
        itemdate mActualStart;
        itemdate mActualEnd;      
        std::string mBlockedBy;
};


#endif
