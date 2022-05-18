#include "scheduler.h"
#include "settings.h"

namespace scheduler
{
    
    void scheduler::outputHTML_High_Level_Gantt2(std::ostream &ofs) const
    {
        std::ostringstream oss;

        oss << R"(
    <script src="frappe-gantt.min.js"></script>
    <link rel="stylesheet" href="frappe-gantt.css">

        


    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript">
    google.charts.load('current', {'packages':['gantt']});
    google.charts.setOnLoadCallback(drawChart);

    function daysToMilliseconds(days) {
      return days * 24 * 60 * 60 * 1000;
    }

    function drawChart() {
        var data = new google.visualization.DataTable();
        data.addColumn('string',    'Task ID');
        data.addColumn('string',    'Task Name');
        data.addColumn('string',    'Resource');
        data.addColumn('date',      'Start Date');
        data.addColumn('date',      'End Date');
        data.addColumn('number',    'Duration');
        data.addColumn('number', 'Percent Complete');
        data.addColumn('string',    'Dependencies');

        data.addRows([
    )";

        bool first = true;
        for (auto &p : mProjects)
        {
            if (!first)
                oss << ", " << std::endl;
            first = false;
            oss << "['" << __protect(p.getName()) << "', '" << __protect(p.getName()) << "', '" << __protect(p.getId()) << "', "
                << p.mActualStart.getAsGoogleNewDate() << ", "
                << p.mActualEnd.getAsGoogleNewDate() << ", "
                << "null, 0, null ]";
        }
        oss << R"(
        ]);
    
    var options = {
        height: 5000,
        gantt: {
            sortTasks: false,
            labelMaxWidth: 400
        }
      };

      var chart = new google.visualization.Gantt(document.getElementById('chart_div'));

      chart.draw(data, options);
    }
  </script>
    )";

        HTMLheaders(ofs, oss.str());
        ofs << R"(
        <h2>High-Level Gantt Chart</h2>
        <div id="chart_div"></div>
        )";

        HTMLfooters(ofs);
    }




    void scheduler::outputHTML_High_Level_Gantt(std::ostream &ofs) const
    {
        std::ostringstream oss;

        oss << R"(
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript">
    google.charts.load('current', {'packages':['gantt']});
    google.charts.setOnLoadCallback(drawChart);

    function daysToMilliseconds(days) {
      return days * 24 * 60 * 60 * 1000;
    }

    function drawChart() {
        var data = new google.visualization.DataTable();
        data.addColumn('string',    'Task ID');
        data.addColumn('string',    'Task Name');
        data.addColumn('string',    'Resource');
        data.addColumn('date',      'Start Date');
        data.addColumn('date',      'End Date');
        data.addColumn('number',    'Duration');
        data.addColumn('number', 'Percent Complete');
        data.addColumn('string',    'Dependencies');

        data.addRows([
    )";

        bool first = true;
        for (auto &p : mProjects)
        {
            if (!first)
                oss << ", " << std::endl;
            first = false;
            oss << "['" << __protect(p.getName()) << "', '" << __protect(p.getName()) << "', '" << __protect(p.getId()) << "', "
                << p.mActualStart.getAsGoogleNewDate() << ", "
                << p.mActualEnd.getAsGoogleNewDate() << ", "
                << "null, 0, null ]";
        }
        oss << R"(
        ]);
    
    var options = {
        height: 5000,
        gantt: {
            sortTasks: false,
            labelMaxWidth: 400
        }
      };

      var chart = new google.visualization.Gantt(document.getElementById('chart_div'));

      chart.draw(data, options);
    }
  </script>
    )";

        HTMLheaders(ofs, oss.str());
        ofs << R"(
        <h2>High-Level Gantt Chart</h2>
        <div id="chart_div"></div>
        )";

        HTMLfooters(ofs);
    }


//---------------------------------------------------------------------------------------------

    // https://developers.google.com/chart/interactive/docs/gallery/ganttchart
    void scheduler::outputHTML_Detailed_Gantt(std::ostream &ofs) const
    {
        std::ostringstream oss;

        oss << R"(
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript">
    google.charts.load('current', {'packages':['gantt']});
    google.charts.setOnLoadCallback(drawChart);

    function daysToMilliseconds(days) {
      return days * 24 * 60 * 60 * 1000;
    }

    function drawChart() {
        var data = new google.visualization.DataTable();
        data.addColumn('string',    'Task ID');
        data.addColumn('string',    'Task Name');
        data.addColumn('string',    'Resource');
        data.addColumn('date',      'Start Date');
        data.addColumn('date',      'End Date');
        data.addColumn('number',    'Duration');
        data.addColumn('number', 'Percent Complete');
        data.addColumn('string',    'Dependencies');

        data.addRows([
    )";

        std::vector<int> v_sorted;
        prioritySortArray(v_sorted);

        bool first = true;
        for (auto &x : v_sorted)
        {
            auto &z = mItems[x];
            if (!first)
                oss << ", " << std::endl;
            first = false;

            // google can't handle milestones.
            workdate tend = z.mActualEnd;
            if (tend == z.mActualStart)
                tend.incrementWorkDay();

            oss << "['" << __protect(z.mId) << "', '" << __protect(z.getFullName()) << "', '" << __protect(mProjects[z.mProject].getId()) << "', "
                << z.mActualStart.getAsGoogleNewDate() << ", "
                << tend.getAsGoogleNewDate() << ", "
                << "null, 0, null ]";
        }

        oss << R"(
        ]);
    
    var options = {
        height: 20000,
        gantt: {
            sortTasks: false,
            labelMaxWidth: 400
        }
      };

      var chart = new google.visualization.Gantt(document.getElementById('chart_div'));

      chart.draw(data, options);
    }
  </script>
    )";

        HTMLheaders(ofs, oss.str());
        ofs << R"(
        <h2>Detailed Gantt Chart</h2>
        <div id="chart_div"></div>
        )";

        HTMLfooters(ofs);
    }

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
                for (unsigned long ri = 0; ri < x.mResources.size(); ++ri)
                    lo.write(S() << getPersonIndexFromName(x.mResources[ri].mName) + 1 << ":" << (x.mLoadingPercent[ri]));
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

} // namespace