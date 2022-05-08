#include <algorithm>
#include <cctype>
#include <string>
#include <iomanip>  
#include <sstream>
#include <numeric>

#include "backlog.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

//-----------------------------------------------------------------------------------------



backlog::backlog(projects & p, const teams & t) : mTeams(t), mProjects(p)
{
    mTeamsItems.resize(mTeams.size());

    for (unsigned int i=0; i<t.size() ;++i)
        {
            unsigned int j=1;
            std::string filename="team-"+makelower(t[i].mId) + ".csv";
            simplecsv teambacklog(filename,true,10);
            if (teambacklog.openedOkay())
            {
                std::vector<std::string> items;
                while (teambacklog.getline(items,10))
                {
                    backlogitem b(items,i,p);
                        {
                        unsigned int n = getItemIndexFromId(b.mId);
                        if (n!=eNotFound)
                            TERMINATE(S()<<"Duplicate ID " << b.mId<<" of two backlog items:"<<std::endl<<b.mDescription<<std::endl<<mItems[n].mDescription);
                        }
                    mTeamsItems[b.mTeamNdx].push_back( b );
                    ++j;
                }
            }
            else    
                std::cout << "Unable to open team csv: "<<filename<<std::endl;
        }

    // copy people name/maxtime from the teams list into our people list.
    for (const auto & x : mTeams)
        for (const auto & y : x.mMembers)
            mPeople.push_back( person( y ));


    // load output types.

    mOutputWriters = {
        outputfilewriter("backlog.txt",         kFile_Text, &backlog::displaybacklog),
        outputfilewriter("people.txt",          kFile_Text, &backlog::displaypeople),
        outputfilewriter("gantt.csv",           kFile_CSV,  &backlog::save_gantt_project_file),
        outputfilewriter("milestones.txt",      kFile_Text, &backlog::displaymilestones),
        outputfilewriter("raw_backlog.txt",     kFile_Text, &backlog::displaybacklog_raw),
        outputfilewriter("projects.txt",        kFile_Text, &backlog::displayprojects),
        outputfilewriter("index.html",          kFile_HTML, &backlog::outputHTML_Index),
        outputfilewriter("people.html",         kFile_HTML, &backlog::outputHTML_People),
        outputfilewriter("costdashboard.html",  kFile_HTML, &backlog::outputHTML_Dashboard),
        outputfilewriter("highlevelgantt.html", kFile_HTML, &backlog::outputHTML_High_Level_Gantt),
        outputfilewriter("detailedgantt.html",  kFile_HTML, &backlog::outputHTML_Detailed_Gantt),
        outputfilewriter("rawbacklog.html",     kFile_HTML, &backlog::outputHTML_RawBacklog)       
    };

}

void backlog::createAllOutputFiles() const
{
    create_output_directories();

    for (auto & f : mOutputWriters)
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
        std::ofstream ofs(p+f.mFileName);
        (this->*f.mFuncPtr)(ofs);
        ofs.close();    
    }
}

void backlog::create_output_directories() const
{
    std::string po =  gSettings().getRoot()+"/output/";
    checkcreatedirectory(po);
    checkcreatedirectory(getOutputPath_Txt());
    checkcreatedirectory(getOutputPath_Csv());
    checkcreatedirectory(getOutputPath_Html());
}


void backlog::prioritySortArray( std::vector<int> & v ) const
{
    // sort by project priority to make a little more readable. :-) 
    v.resize(mItems.size());
    std::iota(v.begin(), v.end(), 0);
    std::sort(begin(v), end(v), [this](int index_left, int index_right) { 
        if (mItems[index_left].mProject != mItems[index_right].mProject)
            return mItems[index_left].mProject < mItems[index_right].mProject; 
        else
            if (mItems[index_left].mActualEnd != mItems[index_right].mActualEnd)
                return mItems[index_left].mActualEnd < mItems[index_right].mActualEnd;
            else
            {
                if (mItems[index_left].mDevDays==0) return false;
                if (mItems[index_right].mDevDays==0) return true; 
                return index_left<index_right;
            }
        });//Ascending order.
}


void vmove(std::vector<backlogitem> & v, const std::size_t i_old, const std::size_t i_new)
{
    auto it = v.begin();
    std::rotate( it + i_new, it + i_old, it + i_old + 1);
}

unsigned int backlog::getItemIndexFromId(const std::string id) const
{
    if (id.length()==0)
        return eNotFound;
    
    for (unsigned int k=0;k<mItems.size();++k)
        if (iSame(mItems[k].mId,id) )
            return k;

    return eNotFound;
}


person & backlog::getPersonByName(const std::string name) 
{   
    for (auto & p : mPeople)
        if (iSame(name,p.mName))
            return p;

    TERMINATE( "Unable to locate resource " + name + " in any team.");
    return mPeople[0];
}

unsigned int backlog::getPersonIndexFromName(const std::string name) const
{
    for (unsigned int i=0;i<mPeople.size();++i)
        if (iSame(mPeople[i].mName, name)) return i;
    TERMINATE("Unable to find name "+name);
    return 0;
}



std::string quoted(std::string s)
{
    return std::string("\"") + s + std::string("\"");
}



// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------



void backlog::schedule()
{
    std::cout << "Scheduling backlog items..."<<std::endl;

    timer t;

    _schedule();
    _calc_project_summary();

    std::cout << "Scheduling done in " << std::setprecision(3) << t.stop() << " ms." << std::endl;
}

// -----------------------------------------------------------------------------------------------------------------

void printorder(const std::vector<backlogitem> & v)
{
    for (auto & z : v) std::cout<<z.mId<<" ";
    std::cout<<std::endl;
}



void backlog::_calc_project_summary()
{ // scheduling done.
    for (auto & z : mProjects)
    {
        z.mTotalDevDays=0.0;
        z.mActualStart.setForever();
        z.mActualEnd.setToStart();
    }
    
    // iterate through tasks, taking max and min duration.
    for (const auto & task : mItems)
    {
        auto & p = mProjects[task.mProject];

        if (task.mActualStart < p.mActualStart)
            p.mActualStart=task.mActualStart;

        if (task.mActualEnd > p.mActualEnd)
            p.mActualEnd=task.mActualEnd;

        if (task.mDependencies.size()==0)
            p.mTotalDevDays += task.mDevDays;
        else
            for (auto & x : task.mResources)
                p.mTotalDevDays += task.getDuration().getAsDurationDouble() *x.mLoadingPercent/100.0;
    }

    for (auto & proj : mProjects)
        if (proj.mActualStart.isForever()) // no tasks.
            proj.mActualStart.setToStart();
}

// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------



