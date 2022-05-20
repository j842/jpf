#include <algorithm>
#include <cctype>
#include <string>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <chrono>

#include "scheduler.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"
#include "colours.h"

namespace scheduler
{

    void scheduler::writelist(std::ostream &oss, const std::vector<std::string> &v)
    {
        for (unsigned int j = 0; j < v.size(); ++j)
        {
            if (j > 0)
                oss << ";";
            oss << v[j];
        }
        oss << ",";
    }

    outputfilewriter::outputfilewriter(std::string fname, tOutputTypes outputType, tFuncPtr fptr) : mFileName(fname), mOutputType(outputType), mFuncPtr(fptr)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------

    void scheduler::displaybacklog_raw(std::ostream &ofs) const
    {
        ofs << std::setw(7) << "ID"
            << " ";
        ofs << std::setw(12) << "Team"
            << "  ";
        ofs << std::setw(6) << "Start"
            << " -> " << std::setw(6) << "End  ";
        ofs << std::setw(10) << "Blocked"
            << " ";
        ofs << std::setw(4) << "Rank"
            << "   ";
        ofs << "TaskName"
            << " { Dependencies }" << std::endl
            << std::endl;

        for (auto &z : mItems)
        {
            ofs << std::setw(7) << z.mId << " ";
            ofs << std::setw(12) << teams().at(z.mTeamNdx).mId << "  ";
            ofs << z.mActualStart.getStr_short() << " -> " << z.mActualEnd.getStr_short();
            ofs << std::setw(10) << z.mBlockedBy << " ";
            ofs << std::setw(4) << z.mPriority << "   ";
            ofs << z.getFullName() << " { ";

            for (auto r : z.mDependencies)
                ofs << r << " ";
            ofs << "}" << std::endl;
        }
    }

    // CSV file.
    void scheduler::displayworkchunks(std::ostream & ofs) const
    {
        std::vector<std::string> row;
        row.push_back("Day");
        row.push_back("Project");
        row.push_back("Item");
        row.push_back("Person");
        row.push_back("DCD Chunk");  
        row.push_back("DCD ItemRemaining");  
        row.push_back("DCD ItemTotal");  
        row.push_back("DCD Person Remaining");  
        simplecsv::output(ofs,row);
        ofs << std::endl;

        std::vector<tCentiDay> itemDevCentiDone(mItems.size(), 0);
        for (auto & c : mWorkLog)
            {
                row.clear();
                row.push_back(c.day.getStr());
                row.push_back(mItems[c.itemNdx].mProjectName);
                row.push_back(mItems[c.itemNdx].mDescription);
                row.push_back(mPeople[c.personNdx].mName);
                row.push_back(S() << c.chunkEffort);
                row.push_back(S() << c.itemRemaining);
                row.push_back(S() << mItems[c.itemNdx].mDevCentiDays);
                row.push_back(S() << c.personDayRemaining);
                simplecsv::output(ofs,row);
                ofs << std::endl;
            }
    }

    void scheduler::displaybacklog(std::ostream &ofs) const
    {
        std::vector<int> v_sorted;
        prioritySortArray(v_sorted);

        unsigned int prevproj = UINT_MAX;
        //    unsigned int c=0;

        unsigned int maxtitlew = std::max(teams().getMaxTeamNameWidth() + 22, projects().getMaxProjectNameWidth());
        unsigned int maxnamew = mPeople.getMaxNameWidth();
        for (auto &x : v_sorted)
        {
            auto &z = mItems[x];

            if (z.mProject != prevproj)
            {
                std::string pname = mProjects[z.mProject].getName();

                ofs << std::endl
                    << std::endl
                    << CENTERSTREAM(pname, maxtitlew)
                    << std::endl; // blank line between projects.
                std::string s(maxtitlew, '-');
                ofs << s << std::endl;
                prevproj = z.mProject;
            }
            // ofs << std::setw(7) << z.mId << " ";

            ofs << RIGHTSTREAM(teams().at(z.mTeamNdx).mId, teams().getMaxTeamNameWidth()) << "  ";
            ofs << z.mActualStart.getStr() << " -> " << z.mActualEnd.getStr();
            ofs << "  " << RIGHTSTREAM(z.mBlockedBy, maxnamew) << "  ";
            ofs << z.mDescription << std::endl;
        }
    }

    void scheduler::displaypeople(std::ostream &ofs) const
    {
        for (auto &z : mPeople)
        {
            ofs << std::endl
                << z.mName << " [ " << std::setw(3) << z.getMaxAvialability().getL() << "% max ]" << std::endl;
            ofs << std::string(z.mName.size() + 13, '-') << std::endl;

            for (auto &j : mItems)
                for (unsigned long pi = 0; pi<j.mResources.size();++pi)
                {
                    auto &p = j.mResources[pi];
                    if (iSame(p.mName, z.mName))
                    {
                        tCentiDay utilisation = 100;
                        if (z.getMaxAvialability() > 0)
                            utilisation = j.mLoadingPercent[pi];

                        std::ostringstream oss;

                        oss << " " << j.mActualStart.getStr() << " to "
                            << std::setw(7) << j.mActualEnd.getStr() << "  [ "
                            << std::setw(3) << utilisation << "% ]  :  ";

                        ofs << oss.str() << j.getFullName() << std::endl;
                    }
                }
        }
    }

    void scheduler::displaymilestones(std::ostream &ofs) const
    {
        std::vector<int> v_sorted;
        prioritySortArray(v_sorted);
        unsigned int prevproj = UINT_MAX;
        for (auto &x : v_sorted)
        {
            auto &z = mItems[x];

            if (z.mDevCentiDays == 0 && z.mMinCalendarDays == 0)
            { // output all 0 day items.
                if (prevproj != z.mProject)
                {
                    ofs << std::endl
                        << mProjects[z.mProject].getId() << std::endl; // blank line between projects.
                    std::string s(mProjects[z.mProject].getId().length(), '-');
                    ofs << s << std::endl;
                    prevproj = z.mProject;
                }
                ofs << z.mActualStart.getStr_short() << "   ";
                ofs << z.mDescription << std::endl;
            }
        }
        ofs << std::endl;
    }

    void scheduler::displayprojects(std::ostream &ofs) const
    {
        std::vector<std::vector<std::string>> table;

        table.push_back({"Project Id", "Start Date", "End Date", "Remaining Cost"});

        for (auto p : mProjects)
        {
            table.push_back({p.getId(),
                             p.mActualStart.getStr(),
                             p.mActualEnd.getStr(),
                             (S() << getDollars( 0.01 * p.mTotalDevCentiDays.getL() * gSettings().dailyDevCost()))});
        }
        _displaytable(ofs, table, " | ",false);
    }


    void scheduler::displayprojects_Console() const
    {
        std::ostream & ofs = std::cout;
        ofs << std::endl << std::endl;

        std::vector<std::vector<std::string>> table;

        table.push_back({"Project Id", "Start Date", "End Date", "Remaining Cost"});

        for (auto p : mProjects)
        {
            table.push_back({p.getId(),
                             p.mActualStart.getStr(),
                             p.mActualEnd.getStr(),
                             (S() << getDollars( 0.01 * p.mTotalDevCentiDays.getL() * gSettings().dailyDevCost()))});
        }
        _displaytable(ofs, table, " | ",true);

        ofs << std::endl << std::endl;
    }



    void scheduler::_displaytable(std::ostream &ofs, std::vector<std::vector<std::string>> &vvs, std::string sepChar, bool consoleColour) const
    {
        using namespace colours;

        if (vvs.size() == 0)
            return;
        if (vvs[0].size() == 0)
            return;
        std::vector<std::size_t> columnwidths(vvs[0].size(), 0);

        for (auto &row : vvs)
        {
            ASSERT(row.size() == columnwidths.size());
            for (unsigned int i = 0; i < row.size(); ++i)
                columnwidths[i] = std::max(columnwidths[i], row[i].length());
        }

        for (unsigned int rowndx = 0; rowndx < vvs.size(); ++rowndx)
        {
            auto &row = vvs[rowndx];

            std::string rowcol = "", sepcol = "";
            if (consoleColour) 
            {   
                sepcol = cLightOrange;
                rowcol = (rowndx>0 ? (rowndx%2==0 ? cLime : cBlue ): sepcol);
            }

            for (unsigned int i = 0; i < row.size(); ++i)
            {
                if (i > 0)
                    ofs << sepcol << sepChar;
                ofs << rowcol << std::setw(columnwidths[i] + 2) << row[i];
            }
            ofs << std::endl;
            if (rowndx == 0)
            {
                for (unsigned int i = 0; i < row.size(); ++i)
                {
                    if (i > 0)
                         ofs << sepcol << sepChar << rowcol;;
                    ofs << std::setw(columnwidths[i] + 2) << std::string(columnwidths[i] + 2, '-');
                }
            ofs << std::endl;
            }
        }

        if (consoleColour)
            ofs << cNoColour;
    }

} // namespace