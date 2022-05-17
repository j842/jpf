#include <algorithm>
#include <cctype>
#include <string>
#include <iomanip>
#include <sstream>
#include <numeric>

#include "scheduler.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"
#include "inputfiles_publicholidays.h"
#include "inputfiles_projects.h"
#include "inputfiles_inputset.h"

//-----------------------------------------------------------------------------------------

namespace scheduler
{


    scheduleditem::scheduleditem(const inputfiles::backlogitem &bli, unsigned int priority,  unsigned int projectndx, unsigned int itemIndexInTeamBacklog) : 
        inputfiles::backlogitem(bli), mProject(projectndx), mPriority(priority),mItemIndexInTeamBacklog(itemIndexInTeamBacklog), 
        mLoadingPercent(bli.mResources.size(),0),
        mTotalContribution(bli.mResources.size(),0)
    {
    }

    unsigned long scheduleditem::getDurationDays() const 
    { 
        return workdate::countWorkDays(mActualStart,mActualEnd);
    }

    scheduledproject::scheduledproject(const inputfiles::project &prj) : inputfiles::project(prj), mTotalDevCentiDays(0)
    {

    }


//-----------------------------------------------------------------------------------------


    scheduler::scheduler(inputfiles::constinputset ifiles) : mScheduled(false), mI(ifiles)
    {
        // load output types.
        mOutputWriters = {
            outputfilewriter("backlog.txt", kFile_Text, &scheduler::displaybacklog),
            outputfilewriter("people.txt", kFile_Text, &scheduler::displaypeople),
            outputfilewriter("gantt.csv", kFile_CSV, &scheduler::save_gantt_project_file),
            outputfilewriter("workchunks.csv", kFile_CSV, &scheduler::displayworkchunks),
            outputfilewriter("milestones.txt", kFile_Text, &scheduler::displaymilestones),
            outputfilewriter("raw_backlog.txt", kFile_Text, &scheduler::displaybacklog_raw),
            outputfilewriter("projects.txt", kFile_Text, &scheduler::displayprojects),
            outputfilewriter("index.html", kFile_HTML, &scheduler::outputHTML_Index),
            outputfilewriter("people.html", kFile_HTML, &scheduler::outputHTML_People),
            outputfilewriter("costdashboard.html", kFile_HTML, &scheduler::outputHTML_Dashboard),
            outputfilewriter("highlevelgantt.html", kFile_HTML, &scheduler::outputHTML_High_Level_Gantt),
            outputfilewriter("detailedgantt.html", kFile_HTML, &scheduler::outputHTML_Detailed_Gantt),
            outputfilewriter("raw_backlog.html", kFile_HTML, &scheduler::outputHTML_RawBacklog)};
    }

    void scheduler::createAllOutputFiles() const
    {
        create_output_directories();

        for (auto &f : mOutputWriters)
        {
            std::string p;
            switch (f.mOutputType)
            {
            case kFile_Text:
                p = getOutputPath_Txt();
                break;
            case kFile_HTML:
                p = getOutputPath_Html();
                break;
            case kFile_CSV:
                p = getOutputPath_Csv();
                break;
            default:
                TERMINATE("Bad code, invalid output type.");
            };
            std::ofstream ofs(p + f.mFileName);
            (this->*f.mFuncPtr)(ofs);
            ofs.close();
        }
    }

    void scheduler::create_output_directories() const
    {
        std::string po = gSettings().getRoot() + "/output/";
        checkcreatedirectory(po);
        checkcreatedirectory(getOutputPath_Txt());
        checkcreatedirectory(getOutputPath_Csv());
        checkcreatedirectory(getOutputPath_Html());
    }

    void scheduler::prioritySortArray(std::vector<int> &v) const
    {
        // sort by project priority to make a little more readable. :-)
        v.resize(mItems.size());
        std::iota(v.begin(), v.end(), 0);
        std::sort(begin(v), end(v), [this](int index_left, int index_right)
                  { 
        if (mItems[index_left].mProject != mItems[index_right].mProject)
            return mItems[index_left].mProject < mItems[index_right].mProject; 
        else
            if (mItems[index_left].mActualEnd != mItems[index_right].mActualEnd)
                return mItems[index_left].mActualEnd < mItems[index_right].mActualEnd;
            else
            {
                if (mItems[index_left].mDevCentiDays==0) return false; // return left
                if (mItems[index_right].mDevCentiDays==0) return true;  // return right.
                return index_left<index_right;
            } }); // Ascending order.
    }

    void vmove(std::vector<scheduleditem> &v, const std::size_t i_old, const std::size_t i_new)
    {
        auto it = v.begin();
        std::rotate(it + i_new, it + i_old, it + i_old + 1);
    }

    unsigned int scheduler::getItemIndexFromId(const std::string id) const
    {
        if (id.length() == 0)
            return eNotFound;

        for (unsigned int k = 0; k < mItems.size(); ++k)
            if (iSame(mItems[k].mId, id))
                return k;

        return eNotFound;
    }

    scheduledperson &scheduler::getPersonByName(const std::string name)
    {
        for (auto &p : mPeople)
            if (iSame(name, p.mName))
                return p;

        TERMINATE("Unable to locate resource " + name + " in any team.");
        return mPeople[0];
    }

    unsigned int scheduler::getPersonIndexFromName(const std::string name) const
    {
        for (unsigned int i = 0; i < mPeople.size(); ++i)
            if (iSame(mPeople[i].mName, name))
                return i;
        TERMINATE("Unable to find name " + name);
        return 0;
    }

    std::string quoted(std::string s)
    {
        return std::string("\"") + s + std::string("\"");
    }


    // -----------------------------------------------------------------------------------------------------------------

    void printorder(const std::vector<scheduleditem> &v)
    {
        for (auto &z : v)
            std::cout << z.mId << " ";
        std::cout << std::endl;
    }

    void scheduler::_calc_project_summary()
    { // scheduling done.
        ASSERT(mScheduled);

        for (auto &z : mProjects)
        {
            z.mTotalDevCentiDays = 0;
            z.mActualStart.setForever();
            z.mActualEnd.setToStart();
        }

        // iterate through tasks, taking max and min duration.
        for (const auto &task : mItems)
        {
            auto &p = mProjects[task.mProject];

            if (task.mActualStart < p.mActualStart)
                p.mActualStart = task.mActualStart;

            if (task.mActualEnd > p.mActualEnd)
                p.mActualEnd = task.mActualEnd;

            if (task.mDependencies.size() == 0)
                p.mTotalDevCentiDays += task.mDevCentiDays;
            else
                for (auto &x : task.mTotalContribution)
                    p.mTotalDevCentiDays += x;
        }

        for (auto &proj : mProjects)
            if (proj.mActualStart.isForever()) // no tasks.
                proj.mActualStart.setToStart();
    }

    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------

} // namespace
