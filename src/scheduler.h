#ifndef __BACKLOG_H
#define __BACKLOG_H

#include <string>
#include <vector>

#include "scheduler_person.h"
#include "scheduler_worklog.h"
#include "inputfiles_teambacklogs.h"
#include "inputfiles_projects.h"
#include "inputfiles_inputset.h"
#include "workdate.h"
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
        workdate mActualStart;
        workdate mActualEnd;
        workdate mClosedEnd;
        std::string mBlockedBy;
        unsigned int mItemIndexInTeamBacklog;

        std::vector<tCentiDay> mLoadingPercent;     // index is person (as in mResources)
        std::vector<tCentiDay> mTotalContribution;  // index is person (as in mResources) 
    };

    class scheduledproject : public inputfiles::project
    {
        public:
            scheduledproject(const inputfiles::project &prj);

            // set when project scheduled.
            workdate mActualStart;
            workdate mActualEnd;
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
        kHol = 3,
        kNumItemTypes,
    } tItemTypes;

    class scheduler
    {
    public:
        scheduler(inputfiles::constinputset ifiles);

        void schedule();
        void refresh(inputfiles::inputset & iset); 
        void advance(workdate newStart, inputfiles::inputset & iset) const;

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
        void displayworkchunks(std::ostream &ofs) const;
        void save_gantt_project_file(std::ostream &ofs) const;

        void outputHTML_Index(std::ostream &ofs) const;
        void outputHTML_Dashboard(std::ostream &ofs) const;
        void outputHTML_High_Level_Gantt(std::ostream &ofs) const;
        void outputHTML_High_Level_Gantt2(std::ostream &ofs) const;
        void outputHTML_Detailed_Gantt(std::ostream &ofs) const;
        void outputHTML_People(std::ostream &ofs) const;
        void outputHTML_PeopleEffort(std::ostream & ofs) const;
        void outputHTML_RawBacklog(std::ostream &ofs) const;

    private:
        void _prepare_to_schedule();
        void _schedule();
        void _prioritiseAndMergeTeams();
        void _topological_sort();
        void _topological_visit(int node, std::vector<bool> &tempMarks, std::vector<bool> &permMarks, std::vector<unsigned int> &scheduledList);
        void _determinestart_and_dotask(unsigned int backlogitemNdx);
        void _dotask_v2(unsigned int itemNdx);
        void _dotask_v2_limitedassign(unsigned int itemNdx, tCentiDay &remainTeamToday, std::vector<tCentiDay> &sumCentiDays, std::vector<tCentiDay> &maxCentiDays, tCentiDay &totalDevCentiDaysRemaining, const workdate id);
        void _calc_project_summary();

        void _displaytable(std::ostream &ofs, std::vector<std::vector<std::string>> &vvs, std::string sepChar, bool consoleColour) const;

        void prioritySortArray(std::vector<int> &v) const;

        scheduledperson &getPersonByName(const std::string name); // creates if not present, but checks against teams.
        unsigned int getItemIndexFromId(const std::string id) const;
        unsigned int getPersonIndexFromName(const std::string name) const;

        // HTML helper routines.

        struct rgbcolour
        {
            int r, g, b;
        };
        // rgbcolour heatmap(float minimum, float maximum, float value) const;

        typedef struct
        {
            std::string mId;
            std::string mName;
            rgbcolour mColour;
            tItemTypes mType;
        } tProjectInfo;

        void CalculateDevDaysTally(
            std::vector<std::vector<double>> &DevDaysTally, // [project][month in future]
            std::vector<tProjectInfo>        &ProjectInfo,
            tNdx personNdx = ULONG_MAX
        ) const;
        
        void Graph_Project_Cost(std::ostream &ofs) const;
        void Graph_Person_Project_Cost(std::ostream &ofs, tNdx personNdx) const;
        void Graph_Total_Project_Cost(std::ostream &ofs) const;
        void Graph_BAU(std::ostream &ofs) const;

        void HTMLheaders_Plotly(std::ostream &ofs) const;
        static void HTMLfooters(std::ostream &ofs);
        static void HTMLheaders(std::ostream &ofs, std::string inHead);
        static std::string __protect(std::string s); // remove half quotes.
        static void writelist(std::ostream &oss, const std::vector<std::string> &v);
        std::string ItemType2String(tItemTypes i) const;

    private:
        bool mScheduled;
        scheduledpeople mPeople; // taken from teammembers, but with added fields.
        std::vector<scheduleditem> mItems; // copied from mI.mB.mTeamItems, merged, and scheduled.
        std::vector<scheduledproject> mProjects; // copied from mI.mP, but with start, end dates etc.
        std::vector<worklogitem> mWorkLog; // record of the work done to complete the items (person-day chunks)

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