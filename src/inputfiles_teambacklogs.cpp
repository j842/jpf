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
            TERMINATE("unable to parse backlog item:" + s);

        if (csvitems.size() != 11)
            TERMINATE("Unexpected number of columns in backlog item: " + s);

        _set(csvitems, teamndx);
    }

    backlogitem::backlogitem(const std::vector<std::string> csvitems, const unsigned int teamndx)
    {
        _set(csvitems, teamndx);
    }

    void backlogitem::_set(const std::vector<std::string> csvitems, const unsigned int teamndx)
    {
        mTeamNdx = teamndx;

        ASSERT(csvitems.size() == 10);

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
        mEarliestStart = itemdate(csvitems[5]);

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
        simplecsv::splitcsv(csvitems[8], mDependencies);

        // 9 - comments
        mComments = csvitems[9];
    }

    void backlogitem::output(std::ostream &os) const
    {
        std::vector<std::string> csvitems;
        csvitems.push_back(mProjectName);
        csvitems.push_back(mId);
        csvitems.push_back(mDescription);
        csvitems.push_back(S() << mMinCalendarDays);
        csvitems.push_back(S() << std::setprecision(2) << std::fixed << ((double)mDevCentiDays)/100.0);

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

        r.clear();
        for (auto &i : mDependencies)
            r += (r.length() > 0 ? std::string(", ") + i : i);
        csvitems.push_back(r);

        csvitems.push_back(mComments);

        simplecsv::output(os, csvitems);
        os << std::endl;
    }

    bool backlogitem::hasDependency(std::string d)
    {
        for (auto &ch : mDependencies)
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

    teambacklogs::teambacklogs(const teams & tms)
    {
        // Create mTeamsItems
        mTeamItems.resize(tms.size());
        for (unsigned int i = 0; i < tms.size(); ++i)
        {
            unsigned int j = 1;
            std::string filename = "backlog-" + makelower(tms[i].mId) + ".csv";
            simplecsv teambacklog(filename, true, 10);
            if (teambacklog.openedOkay())
            {
                std::vector<std::string> items;
                while (teambacklog.getline(items, 10))
                {
                    backlogitem b(items, i);
                    {
                        if (b.mId.length()>0 && exists(b.mId))
                            TERMINATE(S() << "Duplicate ID " << b.mId << " of two backlog items:" << std::endl
                                          << b.getFullName() << std::endl
                                          << getItemFromId(b.mId).getFullName());
                    }
                    mTeamItems[b.mTeamNdx].push_back(b);
                    ++j;
                }
            }
            else
                std::cout << "Unable to open team csv: " << filename << std::endl;
        }
    }

    teambacklogs::teambacklogs(const teambacklogs & other)
    {
        mTeamItems = other.mTeamItems; // deep copy since we're POD.
    }



    void teambacklogs::save_team_CSV(std::ostream &os, unsigned int teamNdx) const // output the backlog as a inputtable csv.
    {
        os << "Project,ID,Description,Min Calendar WorkDays,Remaining DevDays Total,Earliest Start,Blocking,Contributing,Depends On,Comments" << std::endl;
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

} // namespace