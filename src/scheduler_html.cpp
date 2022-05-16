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

    void scheduler::CalculateDevDaysTally(
        std::vector<std::vector<double>> &DevDaysTally, // [project][month in future]
        std::vector<std::string> &ProjectLabels,        // [project]
        std::vector<rgbcolour> &Colours,                // [project]
        std::vector<tItemTypes> &BAU                    // [project]
    ) const
    {
        DevDaysTally.resize(mProjects.size() + 3);

        for (auto &proj : mProjects)
            ProjectLabels.push_back(proj.getId());
        ProjectLabels.push_back("Unscheduled Time");
        ProjectLabels.push_back("Other BAU");
        ProjectLabels.push_back("Project Overhead");

        unsigned long maxmonth = 0;
        for (auto &z : mItems)
            if (z.mActualEnd.getMonthIndex() >= maxmonth)
                maxmonth = z.mActualEnd.getMonthIndex() + 1;

        for (auto &p : DevDaysTally)
            p.resize(maxmonth, 0.0);

        unsigned int maxday = itemdate(monthIndex(maxmonth).getLastMonthDay()).getDayAsIndex();
        for (auto &zp : mPeople)
        {
            for (unsigned int dayndx = 0; dayndx < maxday; ++dayndx)
            {
                simpledate d(simpledate::WorkDays2Date(dayndx));
                monthIndex mI(d);
                for (const auto &zc : zp.getChunks(dayndx))
                    DevDaysTally[mItems[zc.mItemIndex].mProject][mI] += ((double)zc.mEffort) / 100.0;
            }
        }

        for (unsigned int m = 0; m < maxmonth; ++m)
        {
            double devdaystotalinmonth = 0.0;
            for (auto &p : DevDaysTally)
                devdaystotalinmonth += p[m];

            double monthcapacity = 0.0;
            double monthbau = 0.0;
            double monthoverhead = 0.0;

            double workingdaysinmonth = monthIndex(m).workingDaysInMonth();
            if (m==0)
                workingdaysinmonth = wdduration( gSettings().startDate(), monthIndex(1).getFirstMonthDay());

            for (auto &worker : mPeople)
            {
                monthcapacity += workingdaysinmonth * ((double)worker.mEFTProject) / 100.0;
                monthbau += workingdaysinmonth * ((double)worker.mEFTBAU) / 100.0;
                monthoverhead += workingdaysinmonth * ((double)worker.mEFTOverhead) / 100.0;
            }
            double monthslack = monthcapacity - devdaystotalinmonth;

            DevDaysTally[mProjects.size() + 0][m] = monthslack;
            DevDaysTally[mProjects.size() + 1][m] = monthbau;
            DevDaysTally[mProjects.size() + 2][m] = monthoverhead;
        }

        BAU.resize(mProjects.size(), kBAU);
        for (unsigned int pi = 0; pi < mProjects.size(); ++pi)
            BAU[pi] = mProjects[pi].getBAU() ? kBAU : kNew;
        BAU.push_back(kUna); // slack - assume new projects.
        BAU.push_back(kBAU); // BAU.
        BAU.push_back(kNew); // overhead.

        Colours.resize(mProjects.size(), rgbcolour{.r = 0, .g = 200, .b = 100});

        colours::ColorGradient heatMapGradient;
        for (unsigned int ci = 0; ci < Colours.size(); ++ci)
        {
            float r, g, b;
            float v = (float)ci / (float)(Colours.size() - 4);
            heatMapGradient.getColorAtValue(v, r, g, b);
            Colours[ci] = rgbcolour{.r = (int)(r * 255.0 + 0.5), .g = (int)(g * 255.0 + 0.5), .b = (int)(b * 255.0 + 0.5)};
        }

        Colours.push_back(rgbcolour{.r = 210, .g = 180, .b = 140}); // slack
        Colours.push_back(rgbcolour{.r = 100, .g = 100, .b = 100}); // BAU
        Colours.push_back(rgbcolour{.r = 200, .g = 200, .b = 200}); // overhead
    }

    void scheduler::Graph_Total_Project_Cost(std::ostream &ofs) const
    {
        std::vector<std::vector<double>> DevDaysTally;
        std::vector<std::string> Labels;
        std::vector<rgbcolour> Colours;
        std::vector<tItemTypes> BAU;
        CalculateDevDaysTally(DevDaysTally, Labels, Colours, BAU);
        if (DevDaysTally.size() == 0)
            return; // no data.

        ofs << R"(
        <h2>Projected Total Salary Cost Remaining</h2>
        <div id="totalprojectcostpie" style="width:auto;height:800;"></div>    
        <script>
        )";

        std::vector<double> vProjectCostRemaining(Labels.size(), 0.0);
        for (unsigned int i = 0; i < Labels.size(); ++i)
            for (double j : DevDaysTally[i])
                vProjectCostRemaining[i] += j;

        {
            ofs << "var colorlist = [";
            bool firstcolor = true;
            for (auto &c : Colours)
            {
                if (!firstcolor)
                    ofs << ", ";
                firstcolor = false;
                ofs << "'rgb(" << c.r << "," << c.g << "," << c.b << ")'";
            }
            ofs << "];" << std::endl;
        }

        ofs << "var datapie = [{" << std::endl;
        {
            listoutput lo(ofs, "values: [", ", ", "]");
            for (auto &i : vProjectCostRemaining)
                lo.write(S() << std::setprecision(3) << gSettings().dailyDevCost() * i);
        }

        ofs << "," << std::endl;
        {
            listoutput lo(ofs, "labels: [", ", ", "]");
            for (auto &i : Labels)
                lo.writehq(i);
        }

        double totalProjectCostRemaining = 0.0;
        for (auto &vpc : vProjectCostRemaining)
            totalProjectCostRemaining += vpc;

        ofs << "," << std::endl;
        {
            listoutput lo(ofs, "text: [", ", ", "]");
            for (unsigned int i = 0; i < Labels.size(); ++i)
                if (vProjectCostRemaining[i] > 0.01 * totalProjectCostRemaining)
                    lo.writehq(S() << Labels[i] << "   " << getDollars(gSettings().dailyDevCost() * vProjectCostRemaining[i]));
                else
                    lo.writehq("");
        }
        // textinfo: "label+percent",

        ofs << R"(,
        type: 'pie',
        textinfo: "text",
        hoverinfo: "label+value+percent",
        automargin: true,
        marker: {
            colors: colorlist
        }
        }];

    var layoutpie = {
        title: 'Projected Salary Cost Remaining ($)',
        margin: {"t": 100, "b": 100, "l": 100, "r": 100},
    };

    Plotly.newPlot('totalprojectcostpie',datapie,layoutpie);
    </script>
        )";
    }

    void scheduler::Graph_Project_Cost(std::ostream &ofs) const
    {
        std::vector<std::vector<double>> DevDaysTally;
        std::vector<std::string> Labels;
        std::vector<rgbcolour> Colours;
        std::vector<tItemTypes> BAU;
        CalculateDevDaysTally(DevDaysTally, Labels, Colours, BAU);
        if (DevDaysTally.size() == 0)
            return; // no data.
        unsigned long devdaysmonths = DevDaysTally[0].size();
        unsigned int maxmonth = std::min(devdaysmonths, gSettings().endMonth() + 1);

        ofs << R"(
        <h2>Projected Salary Costs by Month</h2>
        <div id="stackedbardevdays" style="width:auto;height:600;"></div>    
        <script>
        )";

        for (unsigned int i = 0; i < Labels.size(); ++i)
        {
            ofs << "var trace" << i << " ={" << std::endl;
            {
                listoutput lo(ofs, "x: [", ", ", "], ");
                for (unsigned int m = 0; m < maxmonth; ++m)
                    lo.writehq(monthIndex(m).getString());
            }

            ofs << std::endl;
            {
                listoutput lo(ofs, "y: [", ", ", "], ");
                for (unsigned int m = 0; m < maxmonth; ++m)
                    lo.write(S() << std::setprecision(3) << gSettings().dailyDevCost() * DevDaysTally[i][m]);
            }
            ofs << std::endl
                << "name: '" << Labels[i] << "'," << std::endl;

            auto &c = Colours[i];
            ofs << "marker: { color: 'rgb(" << c.r << ", " << c.g << ", " << c.b << ")' }," << std::endl;

            ofs << "type: 'bar'," << std::endl
                << "};" << std::endl
                << std::endl;
        }

        {
            listoutput lo(ofs, "var data = [", ", ", "];");
            for (auto it = Labels.size(); it > 0; --it)
                lo.write(S() << "trace" << it - 1);
        }
        ofs << std::endl;
        ofs << R"(
        var layout = {
              title: 'Projected Salary Costs ($)',
            xaxis: {tickfont: {
                size: 14,
                color: 'rgb(107, 107, 107)'
                }},
            yaxis: {
                title: 'NZD',
                titlefont: {
                size: 16,
                color: 'rgb(107, 107, 107)'
                },
                tickfont: {
                size: 14,
                color: 'rgb(107, 107, 107)'
                },
            
            },
            barmode: 'stack'
        };
            Plotly.newPlot('stackedbardevdays', data, layout);
        </script>
    )";
    }

    std::string scheduler::ItemType2String(tItemTypes i) const
    {
        if (i == kBAU)
            return "BAU";
        if (i == kNew)
            return "New";
        return "Slack";
    }

    void scheduler::Graph_BAU(std::ostream &ofs) const
    {
        std::vector<std::vector<double>> DevDaysTally; // [project][month]
        std::vector<std::string> Labels;
        std::vector<rgbcolour> Colours;
        std::vector<tItemTypes> BAU;
        CalculateDevDaysTally(DevDaysTally, Labels, Colours, BAU);
        if (DevDaysTally.size() == 0)
            return; // no data.
        unsigned long devdaysmonths = DevDaysTally[0].size();
        unsigned long maxmonth = std::min(devdaysmonths, gSettings().endMonth() + 1);

        std::vector<std::vector<double>> DDT; // [BAU/New][month]
        DDT.resize(3);
        DDT[kBAU].resize(maxmonth, 0.0);
        DDT[kNew].resize(maxmonth, 0.0);
        DDT[kUna].resize(maxmonth, 0.0);

        for (unsigned int pi = 0; pi < DevDaysTally.size(); ++pi)
            for (unsigned int m = 0; m < maxmonth; ++m)
                DDT[BAU[pi]][m] += DevDaysTally[pi][m];

        for (unsigned int m = 0; m < maxmonth; ++m)
        { // make percentages.
            double tot = DDT[kBAU][m] + DDT[kNew][m] + DDT[kUna][m];
            DDT[kBAU][m] = (DDT[kBAU][m] * 100) / tot;
            DDT[kNew][m] = (DDT[kNew][m] * 100) / tot;
            DDT[kUna][m] = 100 - DDT[kBAU][m] - DDT[kNew][m];
        }

        ofs << R"(
        <h2>Projected BAU work versus New Project work</h2>
        <div id="stackedbarBAU" style="width:auto;height:600;"></div>    
        <script>
        )";

        for (unsigned int i = 0; i < 3; ++i)
        {
            ofs << "var trace" << i << " ={" << std::endl;
            {
                listoutput lo(ofs, "x: [", ", ", "], ");
                for (unsigned int m = 0; m < maxmonth; ++m)
                    lo.writehq(monthIndex(m).getString());
            }

            ofs << std::endl;
            {
                listoutput lo(ofs, "y: [", ", ", "], ");
                for (unsigned int m = 0; m < maxmonth; ++m)
                    lo.write(S() << std::setprecision(3) << DDT[i][m]);
            }
            ofs << std::endl
                << "name: '" << ItemType2String(static_cast<tItemTypes>(i)) << "'," << std::endl;

            rgbcolour c;
            if (i == kBAU)
                c = {190, 30, 50};
            if (i == kNew)
                c = {30, 190, 30};
            if (i == kUna)
                c = Colours[Colours.size() - 3];
            ofs << "marker: { color: 'rgb(" << c.r << ", " << c.g << ", " << c.b << ")' }," << std::endl;

            {
                listoutput lo(ofs, "text: [", ", ", "], ");
                for (unsigned int m = 0; m < maxmonth; ++m)
                    lo.write(S() << "'" << ItemType2String(static_cast<tItemTypes>(i)) << " - " << (int)(0.5 + DDT[i][m]) << "%'");
            }
            ofs << std::endl;

            ofs << "type: 'bar'" << std::endl
                << "};" << std::endl
                << std::endl;
        }

        {
            listoutput lo(ofs, "var data = [", ", ", "];");
            for (auto it = 3; it > 0; --it)
                lo.write(S() << "trace" << it - 1);
        }
        ofs << std::endl;
        ofs << R"(
        var layout = {
              title: 'Projected BAU versus New',
            xaxis: {tickfont: {
                size: 14,
                color: 'rgb(107, 107, 107)'
                }},
            yaxis: {
                title: 'Percent',
                titlefont: {
                size: 16,
                color: 'rgb(107, 107, 107)'
                },
                tickfont: {
                size: 14,
                color: 'rgb(107, 107, 107)'
                },
            
            },
            barmode: 'stack'
        };
            Plotly.newPlot('stackedbarBAU', data, layout);
        </script>
    )";
    }

    void scheduler::outputHTML_Index(std::ostream &ofs) const
    {
        HTMLheaders(ofs, "");
        //<div id="firsttable" style="width:800px;height:250px;"></div>

        ofs << "<h2>Project Backlog</h2>Starting from " << simpledate(gSettings().startDate()).getStr_nice_long() << "<br/>" << std::endl;
        ofs << "<PRE>" << std::endl;
        scheduler::displaybacklog(ofs);
        ofs << "</PRE>" << std::endl;
        HTMLfooters(ofs);
    }

    void scheduler::outputHTML_People(std::ostream &ofs) const
    {
        HTMLheaders(ofs, "");
        ofs << "<h2>Tasks by Person</h2>Starting from " << simpledate(gSettings().startDate()).getStr_nice_long() << "<br/>" << std::endl;
        ofs << "<PRE>" << std::endl;
        scheduler::displaypeople(ofs);
        ofs << "</PRE>" << std::endl;
        HTMLfooters(ofs);
    }

    void scheduler::outputHTML_RawBacklog(std::ostream &ofs) const
    {
        HTMLheaders(ofs, "");
        ofs << "<h2>Raw Backlog</h2>Starting from " << simpledate(gSettings().startDate()).getStr_nice_long() << "<br/>" << std::endl;
        ofs << "<PRE>" << std::endl;
        scheduler::displaybacklog_raw(ofs);
        ofs << "</PRE>" << std::endl;
        HTMLfooters(ofs);
    }

    void scheduler::outputHTML_Dashboard(std::ostream &ofs) const
    {
        HTMLheaders_Plotly(ofs);
        //<div id="firsttable" style="width:800px;height:250px;"></div>

        Graph_Project_Cost(ofs);
        Graph_Total_Project_Cost(ofs);
        Graph_BAU(ofs);

        HTMLfooters(ofs);
    }

    void scheduler::HTMLheaders(std::ostream &ofs, std::string inHead)
    {
        ofs << R"(
    <html><head>
    <script type="text/javascript" src="https://livejs.com/live.js"></script>
    )";

        ofs << inHead << std::endl;

        ofs << R"(
    </head><body>
    <h1>)";
        ofs << gSettings().getTitle();
        ofs << R"(</h1>
    <a href="index.html">Project Backlog</a>&nbsp;&nbsp;
    <a href="people.html">People</a>&nbsp;&nbsp;
    <a href="costdashboard.html">Cost Dashboard</a>&nbsp;&nbsp;
    <a href="highlevelgantt.html">High-Level Gantt</a>&nbsp;&nbsp;
    <a href="detailedgantt.html">Detailed Gantt</a>&nbsp;&nbsp;
    <a href="raw_backlog.html">Debug: Raw Backlog</a>&nbsp;&nbsp;
    </br>
 )";
    }

    void scheduler::HTMLheaders_Plotly(std::ostream &ofs) const
    {
        HTMLheaders(ofs, R"( <script src="https://cdn.plot.ly/plotly-2.11.1.min.js"></script> )");
    }

    void scheduler::HTMLfooters(std::ostream &ofs)
    {
        ofs << R"(
    </body></html>
    <!-- auto generated by JPF -->
 )";
    }

    // remove half quotes.
    std::string __protect(std::string s)
    {
        s.erase(std::remove(s.begin(), s.end(), '\''), s.end());
        return s;
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
            oss << "['" << __protect(p.getId()) << "', '" << __protect(p.getId()) << "', '" << __protect(p.getId()) << "', "
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
            itemdate tend = z.mActualEnd;
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

    void scheduler::outputHTMLError(std::string filename, std::string errormsg)
    {
        std::ofstream ofs(filename);
        HTMLheaders(ofs, "");

        ofs << "<pre>" << errormsg << "</pre>";
        HTMLfooters(ofs);
        ofs.close();
    }

} // namespace