#include "scheduler.h"
#include "settings.h"

namespace scheduler
{
    
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

            workdate fin = x.mActualEnd;
            if (fin > x.mActualStart)
                fin.decrementWorkDay();      // gantt project is closed interval.
            ofs << fin.getStrGantt() << ","; // end date

            // gantt project has duration 1 for same day completion.
            ofs << workdate::countWorkDays(x.mActualStart, x.mActualEnd) << ",";

            ofs << "0.0"
                << ",";
            ofs << (int)(0.5 + ((double)(x.mDevCentiDays.getL() * std::max((size_t)1, x.mResources.size()) * gSettings().dailyDevCost())) / 100.0) << ","; // cost
            ofs << ",";

            ofs << ","; // assignments

            ofs << i + 1 << ","; // outline number

            x.writeresourcenames(ofs); // resources

            {
                listoutput lo(ofs, "", ";", "");
                for (unsigned long ri = 0; ri < x.mResources.size(); ++ri)
                    lo.write(S() << getPersonIndexFromName(x.mResources[ri].mName) + 1 << ":" << (x.mLoadingPercent[ri]));
            }

            ofs << ","; // assignments

            ofs << ",,,"; // task colour, web link, notes.
            ofs << std::setfill('0') << std::setw(2) << x.mProjectIndex + 1 << " " << mProjects[x.mProjectIndex].getId();
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

} // namespace