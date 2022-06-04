#include <algorithm>
#include <cctype>
#include <string>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <filesystem>

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
        inputfiles::backlogitem(bli), mProjectIndex(projectndx),mItemIndexInTeamBacklog(itemIndexInTeamBacklog), 
        mPriority(priority),
        mLoadingPercent(bli.mResources.size(),0),
        mTotalContribution(bli.mResources.size(),0)
    {
    }

    unsigned long scheduleditem::getDurationDays() const 
    { 
        return workdate::countWorkDays(mActualStart,mActualEnd);
    }

    workdate scheduleditem::getLastDayWorked() const
    {
        ASSERT(mClosedEnd>=mActualStart);
        return mClosedEnd;
        // ASSERT(mActualEnd>=mActualStart);
        // if (mActualEnd.isForever() || mActualEnd==mActualStart)
        //     return mActualEnd;
        // workdate s = mActualEnd;
        // s.decrementWorkDay();
        // return s;
    }


    scheduledproject::scheduledproject(const inputfiles::project &prj) : inputfiles::project(prj), mTotalDevCentiDays(0)
    {

    }


/*static*/ void scheduler::getOutputWriters(std::vector<outputfilewriter> & writers)
{
    writers.clear();
    writers = {
            outputfilewriter("backlog.txt", kFile_Text, &scheduler::displaybacklog),
            outputfilewriter("people.txt", kFile_Text, &scheduler::displaypeople),
            outputfilewriter("gantt.csv", kFile_CSV, &scheduler::save_gantt_project_file),
            outputfilewriter("workchunks.csv", kFile_CSV, &scheduler::displayworkchunks),
            outputfilewriter("milestones.txt", kFile_Text, &scheduler::displaymilestones),
            outputfilewriter("raw_backlog.txt", kFile_Text, &scheduler::displaybacklog_raw),
            outputfilewriter("projects.txt", kFile_Text, &scheduler::displayprojects)
    };
            // outputfilewriter("index.html", kFile_HTML, &scheduler::outputHTML_Index),
            // outputfilewriter("header.html", kFile_HTML, &scheduler::outputHTML_header),
            // outputfilewriter("footer.html", kFile_HTML, &scheduler::outputHTML_footer),
            // outputfilewriter("people.html", kFile_HTML, &scheduler::outputHTML_People),
            // outputfilewriter("costdashboard.html", kFile_HTML, &scheduler::outputHTML_Dashboard),
            // outputfilewriter("highlevelgantt.html", kFile_HTML, &scheduler::outputHTML_High_Level_Gantt),
            // outputfilewriter("detailedgantt.html", kFile_HTML, &scheduler::outputHTML_Detailed_Gantt),
            // outputfilewriter("raw_backlog.html", kFile_HTML, &scheduler::outputHTML_RawBacklog),
            // outputfilewriter("peopleeffort.html", kFile_HTML, &scheduler::outputHTML_PeopleEffort),
            // outputfilewriter("testfile.html", kFile_HTML, &scheduler::outputHTML_testfile)};
}


//-----------------------------------------------------------------------------------------


    scheduler::scheduler(inputfiles::constinputset ifiles) : mScheduled(false), mI(ifiles)
    {
    }

    void scheduler::createAllOutputFiles() const
    {
        create_output_directories();

        std::vector<outputfilewriter> writers;
        getOutputWriters(writers);
        for (auto &f : writers)
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
            case kFile_Log:
                p = getOutputPath_Log();
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
        checkcreatedirectory(getOutputPath_Base());
        checkcreatedirectory(getOutputPath_Txt());
        checkcreatedirectory(getOutputPath_Csv());
        checkcreatedirectory(getOutputPath_Html());
        checkcreatedirectory(getOutputPath_Log());
    }

    void scheduler::prioritySortArray(std::vector<int> &v) const
    {
        // sort by project priority to make a little more readable. :-)
        v.resize(mItems.size());
        std::iota(v.begin(), v.end(), 0);
        std::sort(begin(v), end(v), [this](int index_left, int index_right)
                  { 
        if (mItems[index_left].mProjectIndex != mItems[index_right].mProjectIndex)
            return mItems[index_left].mProjectIndex < mItems[index_right].mProjectIndex; 
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

    unsigned int scheduler::getProjectIndexFromId(const std::string id) const
    {
        if (id.length()==0)
            return eNotFound;
        for (unsigned int i = 0; i < mProjects.size(); ++i)
            if (iSame(mProjects.at(i).getId(), id) || iSame(mProjects.at(i).getName(),id))
                return i;
        return eNotFound;
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
        return S() << "\"" << s << "\"";
    }


    // -----------------------------------------------------------------------------------------------------------------

    void printorder(const std::vector<scheduleditem> &v)
    {
        for (auto &z : v)
            std::cout << z.mId << " ";
        std::cout << std::endl;
    }

    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------

} // namespace
