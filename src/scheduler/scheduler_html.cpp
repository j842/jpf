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

    void scheduler::getProjectExtraInfo(std::vector<tProjectInfo> &ProjectInfo) const
    {
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
            unsigned int ci = 0;
            for (ci=0; ci < mProjects.size(); ++ci)
            {
                float r, g, b;
                float v = (float)ci / (float)(mProjects.size() - 4);
                colours::turbo_getColour(v, r, g, b);
                ProjectInfo[ci].mColour = rgbcolour{.r = (int)(r * 255.0 + 0.4999), .g = (int)(g * 255.0 + 0.4999), .b = (int)(b * 255.0 + 0.4999)};
            }
            ProjectInfo[ci+0].mColour = rgbcolour{.r = 176, .g = 82 , .b = 205}; // Holidays
            ProjectInfo[ci+1].mColour = rgbcolour{.r = 210, .g = 180, .b = 140}; // slack
            ProjectInfo[ci+2].mColour = rgbcolour{.r = 100, .g = 100, .b = 100}; // BAU
            ProjectInfo[ci+3].mColour = rgbcolour{.r = 200, .g = 200, .b = 200}; // overhead
        }
    }



    void scheduler::CalculateDevDaysTally(
        std::vector<std::vector<double>> &DevDaysTally, // [project][month in future]
        std::vector<tProjectInfo>        &ProjectInfo,
        tNdx personNdx
    ) const
    {
        DevDaysTally.resize(mProjects.size() + kNumItemTypes);
        getProjectExtraInfo(ProjectInfo);

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
                        DevDaysTally[mItems[zc.mItemIndex].mProjectIndex][mI] += ((double)zc.mEffort) / 100.0;
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
                    monthcapacity += (workingdaysinmonth - worker.holidaysInMonth(m)) * ((double)worker.mEFTProject) / 100.0;
                    monthbau += (workingdaysinmonth - worker.holidaysInMonth(m)) * ((double)worker.mEFTBAU) / 100.0;
                    monthoverhead += (workingdaysinmonth - worker.holidaysInMonth(m)) * ((double)worker.mEFTOverhead) / 100.0;
                }
                double monthslack = monthcapacity - monthbau - devdaystotalinmonth;

                DevDaysTally[mProjects.size() + 0][m] = monthsholidays;
                DevDaysTally[mProjects.size() + 1][m] = monthslack;
                DevDaysTally[mProjects.size() + 2][m] = monthbau;
                DevDaysTally[mProjects.size() + 3][m] = monthoverhead;
            }
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

    // remove half quotes.
    std::string scheduler::__protect(std::string s)
    {
        s.erase(std::remove(s.begin(), s.end(), '\''), s.end());
        return s;
    }

void replacestring(std::string & subject, const std::string &search, const std::string &replace)
{
    size_t pos = 0;
    if (search.empty() || subject.empty()) return;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

    void scheduler::outputHTMLError(std::string filename, std::string errormsg)
    {
        std::ofstream ofs(filename);

        replacestring(errormsg,"\n","\n<br/>\n");

        ofs << R"(
    <html><head>
    <script type="text/javascript" src="https://livejs.com/live.js"></script>
    <link rel="stylesheet" href="static/error.css">
    </head><body>
    <h1>)";
        ofs << gSettings().getTitle();
        ofs << R"(
     - ERROR!</h1>)";
    ofs << "<h2>"<< 
    errormsg 
    << "</h2>";

    time_t now = time(0);
    // convert now to string form
    char* date_time = ctime(&now);

    ofs << "<br/><br/><h3>jpf "<< gSettings().getJPFFullVersionStr() <<". Generated "<< date_time <<"</h3>" <<std::endl;
    ofs << R"(
    </body></html>
    <!-- auto generated by JPF -->
    )";

    ofs.close();
    }

} // namespace