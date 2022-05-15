#ifndef __BACKLOG_H
#define __BACKLOG_H

#include <string>
#include <vector>

#include "scheduler_person.h"
#include "inputfiles_teambacklogs.h"
#include "inputfiles_projects.h"
#include "inputfiles_inputset.h"
#include "itemdate.h"
#include "utils.h"


namespace scheduler
{

    class scheduleditem : public inputfiles::backlogitem
    {
    public:
        scheduleditem(const inputfiles::backlogitem &bli, unsigned int priority, unsigned int projectndx, unsigned int itemIndexInTeamBacklog);
        unsigned long getDurationDays() const;

    public:
        // set while scheduling the task.
        unsigned int mProject;
        unsigned int mPriority;
        itemdate mActualStart;
        itemdate mActualEnd;
        std::string mBlockedBy;
        unsigned int mItemIndexInTeamBacklog;
    };

    class scheduledproject : public inputfiles::project
    {
        public:
            scheduledproject(const inputfiles::project &prj);

            // set when project scheduled.
            itemdate mActualStart;
            itemdate mActualEnd;
            tCentiDay mTotalDevCentiDays; // proportional to cost.
    };


    typedef enum {
        kFile_Text,
        kFile_HTML,
        kFile_CSV,
    } tOutputTypes;

    class scheduler;

    typedef void (scheduler::*tFuncPtr)(std::ostream &ofs) const;

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
        kBAU = 0,
        kNew = 1,
        kUna = 2, // unassigned
    } tItemTypes;

    class scheduler
    {
    public:
        scheduler(inputfiles::constinputset ifiles);

        void schedule();
        void refresh(inputfiles::inputset & iset); 
        void advance(itemdate newStart, inputfiles::inputset & iset);

        void createAllOutputFiles() const;
        void displayprojects(std::ostream &ofs) const;
        void displayprojects_Console() const;
        static void outputHTMLError(std::string filename, std::string errormsg);

    private:
        std::vector<outputfilewriter> mOutputWriters;

        void create_output_directories() const;

        void displaybacklog(std::ostream &ofs) const;
        void displaypeople(std::ostream &ofs) const;
        void displaymilestones(std::ostream &ofs) const;
        void displaybacklog_raw(std::ostream &ofs) const;
        void save_gantt_project_file(std::ostream &ofs) const;

        void outputHTML_Index(std::ostream &ofs) const;
        void outputHTML_Dashboard(std::ostream &ofs) const;
        void outputHTML_High_Level_Gantt(std::ostream &ofs) const;
        void outputHTML_Detailed_Gantt(std::ostream &ofs) const;
        void outputHTML_People(std::ostream &ofs) const;
        void outputHTML_RawBacklog(std::ostream &ofs) const;

    private:
        void _prepare_to_schedule();
        void _schedule();
        void _prioritiseAndMergeTeams();
        void _topological_sort();
        void _topological_visit(int node, std::vector<bool> &tempMarks, std::vector<bool> &permMarks, std::vector<unsigned int> &scheduledList);
        void _determinestart_and_dotask(unsigned int backlogitemNdx);
        void _dotask_v2(unsigned int itemNdx);
        void _dotask_v2_limitedassign(unsigned int itemNdx, const tCentiDay maxAllocation, tCentiDay &remainTeamToday, std::vector<tCentiDay> &sumCentiDays, tCentiDay &totalDevCentiDaysRemaining, const itemdate id);
        void _calc_project_summary();

        void _displaytable(std::ostream &ofs, std::vector<std::vector<std::string>> &vvs, std::string sepChar, bool consoleColour) const;

        void prioritySortArray(std::vector<int> &v) const;

        person &getPersonByName(const std::string name); // creates if not present, but checks against teams.
        unsigned int getItemIndexFromId(const std::string id) const;
        unsigned int getPersonIndexFromName(const std::string name) const;

        // HTML helper routines.

        struct rgbcolour
        {
            int r, g, b;
        };
        // rgbcolour heatmap(float minimum, float maximum, float value) const;

        void CalculateDevDaysTally(
            std::vector<std::vector<double>> &DevDaysTally, // [project][month in future]
            std::vector<std::string> &ProjectLabels,        // [project]
            std::vector<rgbcolour> &Colours,                // [project]
            std::vector<tItemTypes> &BAU                    // [project]
        ) const;

        void Graph_Project_Cost(std::ostream &ofs) const;
        void Graph_Total_Project_Cost(std::ostream &ofs) const;
        void Graph_BAU(std::ostream &ofs) const;

        void HTMLheaders_Plotly(std::ostream &ofs) const;
        static void HTMLfooters(std::ostream &ofs);
        static void HTMLheaders(std::ostream &ofs, std::string inHead);

        static void writelist(std::ostream &oss, const std::vector<std::string> &v);
        //static void writeresourcenames(std::ostream &oss, const std::vector<inputfiles::resource> &v);
        std::string ItemType2String(tItemTypes i) const;


    private:
        bool mScheduled;
        people mPeople; // taken from teammembers, but with added fields.
        std::vector<scheduleditem> mItems; // copied from mI.mB.mTeamItems.
        std::vector<scheduledproject> mProjects; // copied from mI.mP.

        void resetSchedule();


    private:
        const inputfiles::constinputset mI;
        const inputfiles::projects & projects() const {return mI.mP;}
        const inputfiles::teams & teams() const {return mI.mT;}
        const inputfiles::publicholidays & holidays() const {return mI.mH;}
        const inputfiles::teambacklogs & teambacklogs() const {return mI.mB;}
    };

} // namespace

#endif