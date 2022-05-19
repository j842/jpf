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
        std::vector<tProjectInfo>        &ProjectInfo,
        tNdx personNdx
    ) const
    {
        DevDaysTally.resize(mProjects.size() + kNumItemTypes);
        ProjectInfo.resize(mProjects.size() + kNumItemTypes);

        // set up project labels.
        {
            unsigned long x = 0;
            for (x=0;x<mProjects.size();++x)
            {
                ProjectInfo[x].mId = mProjects[x].getId();
                ProjectInfo[x].mName = mProjects[x].getName();
            }
            ProjectInfo[x+0].mId="Holidays";
            ProjectInfo[x+0].mName="Holidays";
            ProjectInfo[x+1].mId="Unscheduled";
            ProjectInfo[x+1].mName="Unscheduled Time";
            ProjectInfo[x+2].mId="Other BAU";
            ProjectInfo[x+2].mName="Non-project BAU work";
            ProjectInfo[x+3].mId="Overhead";
            ProjectInfo[x+3].mName="New Project Overhead";
        }

        // determine the maximum month to consider.
        unsigned long maxmonth = 0;
        {
            for (auto &z : mItems)
                if (z.mActualEnd.getMonthIndex() >= maxmonth)
                    maxmonth = z.mActualEnd.getMonthIndex() + 1;

            maxmonth = std::min(maxmonth, gSettings().endMonth() + 1);
        }

        // determine DevDaysTally
        {
            for (auto &p : DevDaysTally)
                p.resize(maxmonth, 0.0);

            unsigned int maxday = workdate(monthIndex(maxmonth).getLastMonthDay()).getDayAsIndex();
            tNdx ns=0,ne=mPeople.size();
            { // set limits on iteration of people.
                ASSERT(personNdx<mPeople.size() || personNdx==ULONG_MAX);
                if (personNdx!=ULONG_MAX)
                    {
                        ns=personNdx;
                        ne=personNdx+1;
                    }
            }

            // calculate the devdays per project per month
            for (tNdx ndx =ns;ndx < ne; ++ ndx)
            {
                auto &zp = mPeople[ndx];
                for (unsigned int dayndx = 0; dayndx < maxday; ++dayndx)
                {
                    simpledate d(workdate::WorkDays2Date(dayndx));
                    monthIndex mI(d);
                    for (const auto &zc : zp.getChunks(dayndx))
                        DevDaysTally[mItems[zc.mItemIndex].mProject][mI] += ((double)zc.mEffort) / 100.0;
                }
            }

            // now work out the slack, other BAU and project overhead figures by month and add to devdaystally
            for (unsigned int m = 0; m < maxmonth; ++m)
            {
                double devdaystotalinmonth = 0.0;
                for (auto &p : DevDaysTally)
                    devdaystotalinmonth += p[m];

                double monthsholidays = 0.0;
                double monthcapacity = 0.0;
                double monthbau = 0.0;
                double monthoverhead = 0.0;

                double workingdaysinmonth = monthIndex(m).workingDaysInMonth();
                if (m==0)
                    workingdaysinmonth = workdate::countWorkDays( gSettings().startDate(), monthIndex(1).getFirstMonthDay());

                for (tNdx ndx =ns ;ndx < ne; ++ ndx)
                {
                    auto &worker = mPeople[ndx];

                    monthsholidays += worker.holidaysInMonth(m);
                    monthcapacity += workingdaysinmonth * ((double)worker.mEFTProject) / 100.0;
                    monthbau += workingdaysinmonth * ((double)worker.mEFTBAU) / 100.0;
                    monthoverhead += workingdaysinmonth * ((double)worker.mEFTOverhead) / 100.0;
                }
                double monthslack = monthcapacity - devdaystotalinmonth - monthsholidays;

                DevDaysTally[mProjects.size() + 0][m] = monthsholidays;
                DevDaysTally[mProjects.size() + 1][m] = monthslack;
                DevDaysTally[mProjects.size() + 2][m] = monthbau;
                DevDaysTally[mProjects.size() + 3][m] = monthoverhead;
            }
        }

        // tidy up BAU 
        {
            unsigned int pi = 0;
            for (pi=0; pi < mProjects.size(); ++pi)
                ProjectInfo[pi].mType = mProjects[pi].getBAU() ? kBAU : kNew;
            ProjectInfo[pi+0].mType=kHol; // holidays 
            ProjectInfo[pi+1].mType=kUna; // slack - assume new projects.
            ProjectInfo[pi+2].mType=kBAU; // BAU.
            ProjectInfo[pi+3].mType=kNew; // overhead.
        }

        // and Colours
        {
            colours::ColorGradient heatMapGradient;
            unsigned int ci = 0;
            for (ci=0; ci < mProjects.size(); ++ci)
            {
                float r, g, b;
                float v = (float)ci / (float)(mProjects.size() - 4);
                heatMapGradient.getColorAtValue(v, r, g, b);
                ProjectInfo[ci].mColour = rgbcolour{.r = (int)(r * 255.0 + 0.5), .g = (int)(g * 255.0 + 0.5), .b = (int)(b * 255.0 + 0.5)};
            }
            ProjectInfo[ci+0].mColour = rgbcolour{.r = 176, .g = 82 , .b = 205}; // Holidays
            ProjectInfo[ci+1].mColour = rgbcolour{.r = 210, .g = 180, .b = 140}; // slack
            ProjectInfo[ci+2].mColour = rgbcolour{.r = 100, .g = 100, .b = 100}; // BAU
            ProjectInfo[ci+3].mColour = rgbcolour{.r = 200, .g = 200, .b = 200}; // overhead
        }

        // now drop empty projects
        for (long pi = (long)DevDaysTally.size()-1; pi>=0 ; --pi)
        {
            double tally=0;
            for (auto & mm : DevDaysTally[pi])
                tally += mm;                
            
            if (tally<0.01)
                {
                    DevDaysTally.erase(DevDaysTally.begin()+pi);
                    ProjectInfo.erase(ProjectInfo.begin()+pi);
                }
        }
    }

    void scheduler::Graph_Total_Project_Cost(std::ostream &ofs) const
    {
        std::vector<std::vector<double>> DevDaysTally;
        std::vector<tProjectInfo> ProjectInfo;
        CalculateDevDaysTally(DevDaysTally, ProjectInfo);
        if (DevDaysTally.size() == 0)
            return; // no data.

        ofs << R"(
        <h2>Projected Total Salary Cost Remaining</h2>
        <h3>)";

        ofs << gSettings().startDate().getStr_nice_long() << " - "<< gSettings().endDate().getStr_nice_long();

        ofs << R"(</h3>
        <div id="totalprojectcostpie" style="width:auto;height:800;"></div>    
        <script>
        )";

        std::vector<double> vProjectCostRemaining(DevDaysTally.size(), 0.0);
        for (unsigned int i = 0; i < DevDaysTally.size(); ++i)
            for (double j : DevDaysTally[i])
                vProjectCostRemaining[i] += j;
        {
            ofs << "var colorlist = [";
            bool firstcolor = true;
            for (auto &c : ProjectInfo)
            {
                if (!firstcolor)
                    ofs << ", ";
                firstcolor = false;
                ofs << "'rgb(" << c.mColour.r << "," << c.mColour.g << "," << c.mColour.b << ")'";
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
            for (auto &i : ProjectInfo)
                lo.writehq(i.mName);
        }

        double totalProjectCostRemaining = 0.0;
        for (auto &vpc : vProjectCostRemaining)
            totalProjectCostRemaining += vpc;

        ofs << "," << std::endl;
        {
            listoutput lo(ofs, "text: [", ", ", "]");
            for (unsigned int i = 0; i < ProjectInfo.size(); ++i)
                if (vProjectCostRemaining[i] > 0.01 * totalProjectCostRemaining)
                    lo.writehq(S() << ProjectInfo[i].mId << "   " << getDollars(gSettings().dailyDevCost() * vProjectCostRemaining[i]));
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
        std::vector<tProjectInfo> ProjectInfo;
        CalculateDevDaysTally(DevDaysTally, ProjectInfo);
        ASSERT(DevDaysTally.size()>0);
        if (DevDaysTally.size() == 0)
            return; // no data.
        unsigned long maxmonth = DevDaysTally[0].size();

        ofs << R"(
        <h2>Projected Salary Costs by Month</h2>
        <div id="stackedbardevdays" style="width:auto;height:600;"></div>    
        <script>
        )";

        for (unsigned int i = 0; i < DevDaysTally.size(); ++i)
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
                << "name: '" << ProjectInfo[i].mName << "'," << std::endl;

            auto &c = ProjectInfo[i].mColour;
            ofs << "marker: { color: 'rgb(" << c.r << ", " << c.g << ", " << c.b << ")' }," << std::endl;

            ofs << "type: 'bar'," << std::endl
                << "};" << std::endl
                << std::endl;
        }

        {
            listoutput lo(ofs, "var data = [", ", ", "];");
            for (auto it = ProjectInfo.size(); it > 0; --it)
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


    void scheduler::Graph_Person_Project_Cost(std::ostream &ofs, tNdx personNdx) const
    {
        std::vector<std::vector<double>> DevDaysTally;
        std::vector<tProjectInfo> ProjectInfo;
        CalculateDevDaysTally(DevDaysTally, ProjectInfo, personNdx);
        if (DevDaysTally.size() == 0)
            return; // no data.

        std::string pCode = mPeople[personNdx].mName;
        removewhitespace(pCode);
        unsigned long maxmonth = DevDaysTally[0].size();

        ofs << std::endl <<
        "<h2 id=\"" << pCode <<"\">" <<  mPeople[personNdx].mName << " : Projected Effort by Month</h2>" << std::endl
        << "<div id=\"stackedbardevdays_"<<pCode<<"\" style=\"width:auto;height:600;\"></div><script>"<<std::endl;    

        for (unsigned int i = 0; i < ProjectInfo.size(); ++i)
        {
            ofs << "var trace_"<<pCode<<"_" << i << " ={" << std::endl;
            {
                listoutput lo(ofs, "x: [", ", ", "], ");
                for (unsigned int m = 0; m < maxmonth; ++m)
                    lo.writehq(monthIndex(m).getString());
            }

            ofs << std::endl;
            {
                listoutput lo(ofs, "y: [", ", ", "], ");
                for (unsigned int m = 0; m < maxmonth; ++m)
                    lo.write(S() << std::setprecision(3) << DevDaysTally[i][m]);
            }
            ofs << std::endl
                << "name: '" << ProjectInfo[i].mName << "'," << std::endl;

            auto &c = ProjectInfo[i].mColour;
            ofs << "marker: { color: 'rgb(" << c.r << ", " << c.g << ", " << c.b << ")' }," << std::endl;

            {
                listoutput lo(ofs, "text: [", ", ", "], ");
                for (unsigned int m = 0; m < maxmonth; ++m)
                    lo.write(S() << "'" << ProjectInfo[i].mId << "'");
            }

            ofs << "type: 'bar'," << std::endl
                << "};" << std::endl
                << std::endl;
        }

        {
            listoutput lo(ofs, S()<<"var data_"<<pCode<<" = [", ", ", "];");
            for (auto it = ProjectInfo.size(); it > 0; --it)
                lo.write(S() << "trace" << "_" << pCode<<"_"<< it - 1);
        }
        ofs << std::endl;
        ofs << "var layout_"<<pCode<< " = { " <<std::endl
<<"              title: '"<<mPeople[personNdx].mName<< R"(\'s Projected Effort (days)',
            xaxis: {tickfont: {
                size: 14,
                color: 'rgb(107, 107, 107)'
                }},
            yaxis: {
                title: 'Effort (days)',
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
            )" << "Plotly.newPlot('stackedbardevdays_"<<pCode<<"', data_"<<pCode<< ", layout_"<<pCode<< R"();
        </script>
    )";
    }



    std::string scheduler::ItemType2String(tItemTypes i) const
    {
        switch (i)
        {
            case kBAU:
                return "BAU";
            case kNew:
                return "New";
            case kUna:
                return "Slack";
            case kHol:
                return "Holiday";
            default:
                break;
        }
        return "ERROR!";
    }

    void scheduler::Graph_BAU(std::ostream &ofs) const
    {
        std::vector<std::vector<double>> DevDaysTally; // [project][month]
        std::vector<tProjectInfo> ProjectInfo;
        CalculateDevDaysTally(DevDaysTally, ProjectInfo);
        if (DevDaysTally.size() == 0)
            return; // no data.
        unsigned long maxmonth = DevDaysTally[0].size();

        std::vector<std::vector<double>> DDT; // [BAU/New][month]
        DDT.resize(kNumItemTypes);
        DDT[kBAU].resize(maxmonth, 0.0);
        DDT[kNew].resize(maxmonth, 0.0);
        DDT[kUna].resize(maxmonth, 0.0);
        DDT[kHol].resize(maxmonth, 0.0);

        for (unsigned int pi = 0; pi < DevDaysTally.size(); ++pi)
            for (unsigned int m = 0; m < maxmonth; ++m)
                DDT[ProjectInfo[pi].mType][m] += DevDaysTally[pi][m];

        for (unsigned int m = 0; m < maxmonth; ++m)
        { // make percentages.
            double tot = DDT[kBAU][m] + DDT[kNew][m] + DDT[kUna][m] + DDT[kHol][m];
            DDT[kBAU][m] = (DDT[kBAU][m] * 100) / tot;
            DDT[kNew][m] = (DDT[kNew][m] * 100) / tot;
            DDT[kHol][m] = (DDT[kHol][m] * 100) / tot;
            DDT[kUna][m] = 100 - DDT[kBAU][m] - DDT[kNew][m] - DDT[kHol][m];
        }

        ofs << R"(
        <h2>Projected BAU work versus New Project work</h2>
        <div id="stackedbarBAU" style="width:auto;height:600;"></div>    
        <script>
        )";

        for (unsigned int i = 0; i < kNumItemTypes; ++i)
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
                c = ProjectInfo[ProjectInfo.size() - 3].mColour;
            if (i==kHol)
                c = ProjectInfo[ProjectInfo.size() - 4].mColour;
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
            for (unsigned long  it = kNumItemTypes; it > 0; --it)
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

    void scheduler::outputHTML_PeopleEffort(std::ostream & ofs) const
    {
        HTMLheaders_Plotly(ofs);
        
        std::ostringstream oss;
        oss<<"<br/><hr/><br/>";
        for (auto & p : mPeople)
        {
            std::string pCode = p.mName;
            removewhitespace(pCode);
            oss << "<a href=\"#" << pCode << "\">"<<p.mName<<"</a>" <<std::endl;
        }
        oss<<"<br/><hr/><br/>";
        oss <<std::endl<<std::endl;

        for (tNdx i=0; i<mPeople.size();++i)
        {
            Graph_Person_Project_Cost(ofs,i);
            ofs << oss.str();
        }
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
    <a href="people.html">People Backlog</a>&nbsp;&nbsp;
    <a href="peopleeffort.html">People Effort</a>&nbsp;&nbsp;
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
        time_t now = time(0);
        // convert now to string form
        char* date_time = ctime(&now);

        ofs << "<br/><br/><font size=\"-1\">jpf "<< gSettings().getJPFFullVersionStr() <<". Generated "<< date_time <<"</font>" <<std::endl;
        ofs << R"(
    </body></html>
    <!-- auto generated by JPF -->
 )";
    }

    // remove half quotes.
    std::string scheduler::__protect(std::string s)
    {
        s.erase(std::remove(s.begin(), s.end(), '\''), s.end());
        return s;
    }

    void scheduler::outputHTMLError(std::string filename, std::string errormsg)
    {
        std::ofstream ofs(filename);
        HTMLheaders(ofs, "");

        ofs << "<h1>ERROR!<h1>" <<std::endl;
        ofs << "<font size=\"+2\">" << errormsg << "</font>";
        HTMLfooters(ofs);
        ofs.close();
    }

} // namespace