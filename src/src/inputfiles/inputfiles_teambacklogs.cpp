#include <experimental/iterator>

#include "inputfiles_teambacklogs.h"

#include "settings.h"
#include "simplecsv.h"
#include "utils.h"
#include "inputfiles_projects.h"
#include "inputfiles_teams.h"

namespace inputfiles
{

    backlogitem::backlogitem(const std::string s, const unsigned int teamndx)
    {
        std::vector<std::string> csvitems;
        if (!simplecsv::splitcsv(s, csvitems))
            TERMINATE(S()<<"unable to parse backlog item:" << s);

        if (csvitems.size() != 11)
            TERMINATE(S()<<"Unexpected number of columns in backlog item: " << s);

        _set(csvitems, teamndx);
    }

    backlogitem::backlogitem(const std::vector<std::string> csvitems, const unsigned int teamndx)
    {
        _set(csvitems, teamndx);
    }

    void backlogitem::_set(const std::vector<std::string> csvitems, const unsigned int teamndx)
    {
        mTeamNdx = teamndx;

        ASSERT(csvitems.size() == 11);

        // 0 - project
        mProjectName = csvitems[0];

        // 1 - ID
        mId = csvitems[1];

        // 2 - Description
        mDescription = csvitems[2];

        // 3 - Min Calendar Workdays
        mMinCalendarDays = str2L(csvitems[3]);

        // 4 - Min Dev Days (TOTAL)
        mDevCentiDays = (int)(0.5 + str2positivedouble(csvitems[4])*100.0);

        // 5 - Earliest Start Date
        mEarliestStart = workdate(csvitems[5]);

        // 6 - blocking resources
        std::vector<std::string> names;
        simplecsv::splitcsv(csvitems[6], names);
        for (auto &name : names)
            mResources.push_back(resource(name, true)); // blocking resources

        // 7 - contributing resources
        names.clear();
        simplecsv::splitcsv(csvitems[7], names);
        for (auto &name : names)
            mResources.push_back(resource(name, false)); // contributing resources

        // 8 - dependencies
        mDependencies = cTags(csvitems[8]);

        // 9 - tags
        mTags = cTags(csvitems[9]);

        // 10 - comments
        mComments = csvitems[10];
    }

    void backlogitem::output(std::ostream &os) const
    {
        std::vector<std::string> csvitems;
        csvitems.push_back(mProjectName);
        csvitems.push_back(mId);
        csvitems.push_back(mDescription);
        csvitems.push_back(S() << mMinCalendarDays);
        csvitems.push_back(S() << std::fixed  << std::setprecision(2) << ((double)mDevCentiDays)/100.0);

        if (mEarliestStart == gSettings().startDate())
            csvitems.push_back("");
        else
            csvitems.push_back(mEarliestStart.getStr());

        std::string r;
        for (auto &i : mResources)
            if (i.mBlocking)
                r += (r.length() > 0 ? std::string(", ") + i.mName : i.mName);
        csvitems.push_back(r);

        r.clear();
        for (auto &i : mResources)
            if (!i.mBlocking)
                r += (r.length() > 0 ? std::string(", ") + i.mName : i.mName);
        csvitems.push_back(r);

        csvitems.push_back(mDependencies.getAsString());

        csvitems.push_back(mTags.getAsString());

        csvitems.push_back(mComments);

        simplecsv::outputr(os, csvitems);
    }

    bool backlogitem::hasDependency(std::string d)
    {
        for (const auto &ch : mDependencies)
            if (iSame(ch, d))
                return true;

        return false;
    }

    std::string backlogitem::getFullName() const
    {
        return mProjectName + " - " + mDescription;
    }

    void backlogitem::writeresourcenames(std::ostream &oss) const
    {
        { // tidied on dtor.
            listoutput lo(oss, "", ";", "");
            for (auto &vv : mResources)
                lo.write(vv.mName);
        }
        oss << ",";
    }

    std::string vec2str(std::vector<std::string> & i)
    {
        std::ostringstream oss;
        std::copy(i.begin(),
              i.end(),
              std::experimental::make_ostream_joiner(oss, ", "));
        return oss.str();
    }

    void teambacklogs::validateBacklogItem(const backlogitem & b,const teams & tms,const projects &pjts, unsigned int taskNum, std::string filename, std::string row) const
    {
        std::string errmsg="";
        if (b.mId.length()>0 && exists(b.mId))
            errmsg += S() << "Duplicate ID \"" << b.mId << "\" with previous backlog item:" << std::endl
                            << getItemFromId(b.mId).getFullName() << std::endl;

        if (pjts.getIndexByID(b.mProjectName)==eNotFound)
            errmsg += S() << "The project name \""<< b.mProjectName<<"\" was not found in projects.csv."<<std::endl;

        if (b.mDevCentiDays>0 && b.mResources.size()==0)
            errmsg += S() << "The task requires development effort, but is assigned to nobody." << std::endl;
        
        if (b.mId.length()==0)
            errmsg += S() << "The task does not have an Id."<< std::endl;

        if (errmsg.length()>0)
            TERMINATE(
                S()<<"Error processing task "<<taskNum<<" from "<<filename<<": "<<std::endl
                << row << std::endl << std::endl
                << errmsg
            );
    }


    teambacklogs::teambacklogs(const teams & tms,const projects & pjts) : mTotalNumItems(0)
    {
        // Create mTeamsItems
        mTeamItems.resize(tms.size());
        for (unsigned int i = 0; i < tms.size(); ++i)
        {
            unsigned int j = 1;
            std::string filename = "backlog-" + makelower(tms[i].mId) + ".csv";
            logdebug("Reading "+filename);
            simplecsv teambacklog(filename, true, 11);
            if (teambacklog.openedOkay())
            {
                std::vector<std::string> items;
                while (teambacklog.getline(items, 11))
                {
                    backlogitem b(items, i);
                    validateBacklogItem(b,tms,pjts,j,filename, vec2str(items));
                    mTeamItems[b.mTeamNdx].push_back(b);
                    ++mTotalNumItems;
                    ++j;
                }
            }
            else
                logwarning(S()<<"Unable to open team csv: " << filename);
        }

        // validate dependencies.
        for (unsigned int i = 0; i < tms.size(); ++i)
            for (auto & bi : mTeamItems[i])
                for (auto & dep : bi.mDependencies)
                { // needs to be either a project or a 
                    depRef dr(dep,*this,pjts);
                    if (dr.getType()==kInvalid)
                    {
                        std::ostringstream oss;
                        bi.output(oss);
                        TERMINATE(
                            S() << "Error processing task from "<<"backlog-" + makelower(tms[i].mId) + ".csv"<<": "<<std::endl
                                << oss.str() << std::endl << std::endl
                                << "The dependency "<<dep<<" is neither a task Id nor a project Id." << std::endl
                        );
                    }
                }
    }

    teambacklogs::teambacklogs(const teambacklogs & other) : mTotalNumItems(0)
    {
        mTeamItems = other.mTeamItems; // deep copy since we're POD.
        mTotalNumItems = other.getTotalNumItems();
    }



    void teambacklogs::save_team_CSV(std::ostream &os, unsigned int teamNdx) const // output the backlog as a inputtable csv.
    {
        os << "Project,ID,Description,Min Calendar WorkDays,Remaining DevDays Total,Earliest Start,Blocking,Contributing,Depends On,Tags,Comments" << std::endl;
        for (auto &i : mTeamItems)
            for (auto & j : i)
                if (j.mTeamNdx == teamNdx)
                    j.output(os);
    }

    bool teambacklogs::exists(std::string id) const
    {
        for (const auto & i : mTeamItems)
            for (const auto & j : i)
                if (iSame(j.mId,id))
                    return true;
        return false;
    }

    const backlogitem & teambacklogs::getItemFromId(std::string id) const
    {
        ASSERT(mTeamItems.size()>0);
        for (const auto & i : mTeamItems)
            for (const auto & j : i)
                if (iSame(j.mId,id))
                    return j;
        TERMINATE("Failed to find backlog item with id "+id);
        return mTeamItems[0][0]; // code never gets here.
    }

    unsigned int teambacklogs::getTotalNumItems() const
    {
        return mTotalNumItems;
    }

    bool backlogitem::hasTag(std::string tag) const
    {
        return mTags.hasTag(tag);
    }

    void backlogitem::copyinto(std::vector<std::string> & tags) const
    {
        mTags.copyinto(tags);
    }



    depRef::depRef(std::string dep, const teambacklogs & tbl, const projects &pjts) : 
        mDep(dep),mProjectNdx(0),mTeamNdx(0),mItemIndexinTeam(0)
    {
        for (unsigned int pi=0;pi<pjts.size();++pi)
            if (iSame(pjts[pi].getId(),dep))
            {
                mType=kProject;
                mProjectNdx = pi;
                return;
            }
        
        for (unsigned int ti=0;ti<tbl.mTeamItems.size();++ti)
            for (unsigned int tim=0;tim<tbl.mTeamItems[ti].size();++tim)
                if (iSame(tbl.mTeamItems[ti][tim].mId,dep))
                    {
                        mType = kTask;
                        mTeamNdx = ti;
                        mItemIndexinTeam = tim;
                        return;
                    }
    }

    tDepType depRef::getType() const 
    {
        return mType;
    }
    unsigned int depRef::getProjectNdx() const
    {
        ASSERT(mType==kProject);
        return mProjectNdx;
    }
    void depRef::getTeamItemNdx(unsigned int & teamndx, unsigned int & itemindexinteam) const
    {
        ASSERT(mType==kTask);
        teamndx = mTeamNdx;
        itemindexinteam = mItemIndexinTeam;
    }
    std::string depRef::getRefString() const
    {
        return mDep;
    }


} // namespace