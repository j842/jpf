#ifndef __BACKLOG_H
#define __BACKLOG_H

#include <string>
#include <vector>

#include "projects.h"
#include "teams.h"
#include "person.h"
#include "itemdate.h"

std::string makelower(const std::string & s);

bool iSame(const std::string &s1, const std::string &s2);

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

class backlogitem
{
    public:
        backlogitem(const std::string s, const unsigned int teamndx, const projects &p);
        backlogitem(const std::vector<std::string> items, const unsigned int teamndx, const projects &p);
        void set(const std::vector<std::string> items, const unsigned int teamndx, const projects &p);

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

        // set while scheduling the task.
        unsigned int mPriority;
        unsigned int mMergePriority;

        // set as part of scheduling.
        itemdate mActualStart;
        itemdate mActualEnd;      
        std::string mBlockedBy;
};

typedef enum
{
    kFile_Text,
    kFile_HTML,
    kFile_CSV,
} tOutputTypes;


class backlog;

typedef void(backlog::*tFuncPtr)(std::ostream & ofs) const;

class outputfilewriter
{
    public: 
        outputfilewriter(std::string fname, tOutputTypes outputType, tFuncPtr fptr);

        std::string mFileName;
        tOutputTypes mOutputType;
        tFuncPtr mFuncPtr;
};

class backlog
{
    public:
        backlog(projects & p, const teams & t);

        void schedule();
        void createAllOutputFiles() const;
        void displayprojects(std::ostream & ofs) const;
        static void outputHTMLError(std::string filename, std::string errormsg);

    private:
        std::vector< outputfilewriter > mOutputWriters;

        void create_output_directories() const;

        void displaybacklog(std::ostream & ofs) const;
        void displaypeople(std::ostream & ofs) const;
        void displaymilestones(std::ostream & ofs) const;
        void displaybacklog_raw(std::ostream & ofs) const;
        void save_gantt_project_file(std::ostream &ofs) const;

        void outputHTML_Index(std::ostream & ofs) const;
        void outputHTML_Dashboard(std::ostream & ofs) const;
        void outputHTML_High_Level_Gantt(std::ostream & ofs) const;
        void outputHTML_Detailed_Gantt(std::ostream & ofs) const;
        void outputHTML_People(std::ostream & ofs) const;
        void outputHTML_RawBacklog(std::ostream & ofs) const;

    private:
        void _prioritiseAndMergeTeams();
        void _schedule();
        void _calc_project_summary();
        void _topological_sort();
        void _topological_visit(int node, std::vector<bool> & tempMarks, std::vector<bool> & permMarks, std::vector<unsigned int> & scheduledList);

        void _displaytable(std::ostream & ofs, std::vector<std::vector<std::string>> & vvs) const;

        void prioritySortArray( std::vector<int> & v ) const;

        person & getPersonByName(const std::string name); //creates if not present, but checks against teams.
        unsigned int getItemIndexFromId(const std::string id) const;
        unsigned int getPersonIndexFromName(const std::string name) const;

        // HTML helper routines.
        
        struct rgbcolour {int r,g,b;};
        //rgbcolour heatmap(float minimum, float maximum, float value) const;

        void CalculateDevDaysTally(
            std::vector< std::vector<double>> & DevDaysTally,   // [project][month in future]
            std::vector< std::string> & ProjectLabels,          // [project]
            std::vector<rgbcolour> & Colours
            ) const; 

        void Graph_Project_Cost(std::ostream & ofs) const;
        void Graph_Total_Project_Cost(std::ostream & ofs) const;
        void HTMLheaders_Plotly(std::ostream & ofs) const;
        static void HTMLfooters(std::ostream & ofs);
        static void HTMLheaders(std::ostream & ofs,std::string inHead);

        static void writelist(std::ostream & oss, const std::vector<std::string> & v);
        static void writeresourcenames(std::ostream & oss, const std::vector<resource> & v);

        std::vector<backlogitem> mItems;
        std::vector<std::deque<backlogitem>> mTeamsItems;

        std::vector<person> mPeople;
        const teams & mTeams;
        projects & mProjects;

};


#endif