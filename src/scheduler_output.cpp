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

    void scheduler::save_gantt_project_file(std::ostream &ofs) const
    {
        ofs << "ID,Name,Begin date,End date,Duration,Completion,Cost,Coordinator,Predecessors,Outline number,Resources,Assignments,Task color,Web Link,Notes,Project" << std::endl;

        std::vector<int> v_sorted;
        prioritySortArray(v_sorted);

        ASSERT(v_sorted.size() == mItems.size());

        for (unsigned int i = 0; i < mItems.size(); ++i)
        {
            auto &x = mItems[v_sorted[i]];

            ofs << i + 1 << ","; // id

            ofs << quoted(x.mDescription) << ","; // name

            ofs << x.mActualStart.getStrGantt() << ","; // being date

            itemdate fin = x.mActualEnd;
            if (fin > x.mActualStart)
                fin.decrement();             // gantt project is closed interval.
            ofs << fin.getStrGantt() << ","; // end date

            // gantt project has duration 1 for same day completion.
            itemdate duration = (x.mActualEnd - x.mActualStart);
            ofs << duration.getAsDurationString() << ",";

            ofs << "0.0"
                << ",";
            ofs << 100 * x.mDevDays * std::max((size_t)1, x.mResources.size()) << ","; // cost
            ofs << ",";

            // dependencies list. Too complicated for GanttProject.
            //  for (unsigned int j=0;j<x.mDependencies.size();++j)
            //  {
            //      if (j>0) ofs << ";";
            //      ofs <<  getItemIndexFromId(x.mDependencies[j])+1;
            //  }
            ofs << ","; // assignments

            ofs << i + 1 << ","; // outline number

            x.writeresourcenames(ofs); // resources

            {
                listoutput lo(ofs, "", ";", "");
                for (auto &y : x.mResources)
                    lo.write(S() << getPersonIndexFromName(y.mName) + 1 << ":" << (int)(0.5 + y.mLoadingPercent));
            }

            ofs << ","; // assignments

            ofs << ",,,"; // task colour, web link, notes.
            ofs << std::setfill('0') << std::setw(2) << x.mProject + 1 << " " << mProjects[x.mProject].getId();
            ofs << std::endl;
        }

        ofs << std::endl;
        ofs << std::endl; // two blank lines

        // resourcing.
        ofs << "ID,Name,Default role,e-mail,Phone,Standard rate,Total cost,Total load" << std::endl;
        for (unsigned int i = 0; i < mPeople.size(); ++i)
        {
            auto &x = mPeople[i];

            ofs << i + 1 << ","; // id.
            ofs << x.mName << ",";
            ofs << "Default:0,";
            ofs << ",,0,0,1.0";
            ofs << std::endl;
        }
    }

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
                std::string pname = mProjects[z.mProject].getId();

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
                << z.mName << " [ " << std::setw(3) << z.getMaxAvialability() << "% max ]" << std::endl;
            ofs << std::string(z.mName.size() + 13, '-') << std::endl;

            for (auto &j : mItems)
                for (auto &p : j.mResources)
                    if (iSame(p.mName, z.mName))
                    {
                        int utilisation = 100;
                        if (z.getMaxAvialability() > 0)
                            utilisation = 0.5 + (double)(p.mLoadingPercent);

                        std::ostringstream oss;

                        oss << " " << j.mActualStart.getStr() << " to "
                            << std::setw(7) << j.mActualEnd.getStr() << "  [ "
                            << std::setw(3) << utilisation << "% ]  :  ";

                        ofs << oss.str() << j.getFullName() << std::endl;
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

            if (z.mDevDays == 0 && z.mMinCalendarDays == 0)
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
                             (S() << "$" << std::setprecision(0) << std::fixed << p.mTotalDevDays * gSettings().dailyDevCost() + 0.5)});
        }
        _displaytable(ofs, table);
    }

    void scheduler::_displaytable(std::ostream &ofs, std::vector<std::vector<std::string>> &vvs) const
    {
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
            for (unsigned int i = 0; i < row.size(); ++i)
            {
                if (i > 0)
                    ofs << " | ";
                ofs << std::setw(columnwidths[i] + 2) << row[i];
            }
            ofs << std::endl;
            if (rowndx == 0)
            {
                for (unsigned int i = 0; i < row.size(); ++i)
                {
                    if (i > 0)
                        ofs << " | ";
                    ofs << std::setw(columnwidths[i] + 2) << std::string(columnwidths[i] + 2, '-');
                }
            }
            ofs << std::endl;
        }
    }

} // namespace