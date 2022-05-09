#ifndef __BACKLOG_H
#define __BACKLOG_H

#include <string>
#include <vector>

#include "projects.h"
#include "teams.h"
#include "person.h"
#include "itemdate.h"
#include "backlogitem.h"
#include "utils.h"

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

typedef enum
{
    kBAU=0,
    kNew=1,
    kUna=2, // unassigned
} tItemTypes;

class backlog
{
    public:
        backlog(projects & p, const teams & t);

        void schedule();
        void createAllOutputFiles() const;
        void displayprojects(std::ostream & ofs) const;
        static void outputHTMLError(std::string filename, std::string errormsg);
        void output(std::ostream & os, unsigned int teamNdx) const; // output the backlog as a inputtable csv for the given team.

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
        void _dotask_v1(backlogitem & z);
        void _determinestart_and_dotask(unsigned int backlogitemNdx);
        void _dotask_v2(backlogitem & z);
        void _dotask_v2_limitedassign(backlogitem & z, const tCentiDay maxAllocation,tCentiDay & remainTeamToday, std::vector<double> & sumCentiDays,tCentiDay & totalDevCentiDaysRemaining,  const itemdate id);

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
            std::vector<rgbcolour> & Colours,                   // [project]
            std::vector<tItemTypes> & BAU                       // [project]
            ) const; 

        void Graph_Project_Cost(std::ostream & ofs) const;
        void Graph_Total_Project_Cost(std::ostream & ofs) const;
        void Graph_BAU(std::ostream & ofs) const;

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