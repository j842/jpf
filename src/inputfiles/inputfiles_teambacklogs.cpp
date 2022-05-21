#include "inputfiles_teambacklogs.h"

#include "settings.h"
#include "simplecsv.h"
#include "utils.h"
#include "inputfiles_projects.h"
#include "inputfiles_teams.h"

namespace inputfiles
{

    backlogitem::backlogitem(const std::wstring s, const unsigned int teamndx)
    {
        std::vector<std::wstring> csvitems;
        if (!simplecsv::splitcsv(s, csvitems))
            TERMINATE(S()<<L"unable to parse backlog item:" << s);

        if (csvitems.size() != 11)
            TERMINATE(S()<<L"Unexpected number of columns in backlog item: " << s);

        _set(csvitems, teamndx);
    }

    backlogitem::backlogitem(const std::vector<std::wstring> csvitems, const unsigned int teamndx)
    {
        _set(csvitems, teamndx);
    }

    void backlogitem::_set(const std::vector<std::wstring> csvitems, const unsigned int teamndx)
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
        mEarliestStart = workdate(csvitems[5]);

        // 6 - blocking resources
        std::vector<std::wstring> names;
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

    void backlogitem::output(std::wostream &os) const
    {
        std::vector<std::wstring> csvitems;
        csvitems.push_back(mProjectName);
        csvitems.push_back(mId);
        csvitems.push_back(mDescription);
        csvitems.push_back(S() << mMinCalendarDays);
        csvitems.push_back(S() << std::fixed  << std::setprecision(2) << ((double)mDevCentiDays)/100.0);

        if (mEarliestStart == gSettings().startDate())
            csvitems.push_back(L"");
        else
            csvitems.push_back(mEarliestStart.getStr());

        std::wstring r;
        for (auto &i : mResources)
            if (i.mBlocking)
                r += (r.length() > 0 ? std::wstring(L", ") + i.mName : i.mName);
        csvitems.push_back(r);

        r.clear();
        for (auto &i : mResources)
            if (!i.mBlocking)
                r += (r.length() > 0 ? std::wstring(L", ") + i.mName : i.mName);
        csvitems.push_back(r);

        r.clear();
        for (auto &i : mDependencies)
            r += (r.length() > 0 ? std::wstring(L", ") + i : i);
        csvitems.push_back(r);

        csvitems.push_back(mComments);

        simplecsv::output(os, csvitems);
        os << std::endl;
    }

    bool backlogitem::hasDependency(std::wstring d)
    {
        for (auto &ch : mDependencies)
            if (iSame(ch, d))
                return true;

        return false;
    }

    std::wstring backlogitem::getFullName() const
    {
        return mProjectName + L" - " + mDescription;
    }

    void backlogitem::writeresourcenames(std::wostream &oss) const
    {
        { // tidied on dtor.
            listoutput lo(oss, L"", L";", L"");
            for (auto &vv : mResources)
                lo.write(vv.mName);
        }
        oss << ",";
    }

    teambacklogs::teambacklogs(const teams & tms) : mTotalNumItems(0)
    {
        // Create mTeamsItems
        mTeamItems.resize(tms.size());
        for (unsigned int i = 0; i < tms.size(); ++i)
        {
            unsigned int j = 1;
            std::string filename = "backlog-" + w2s(makelower(tms[i].mId)) + ".csv";
            simplecsv teambacklog(filename, true, 10);
            if (teambacklog.openedOkay())
            {
                std::vector<std::wstring> items;
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
                    ++mTotalNumItems;
                    ++j;
                }
            }
            else
                logwarning(S()<<"Unable to open team csv: " << filename);
        }
    }

    teambacklogs::teambacklogs(const teambacklogs & other) : mTotalNumItems(0)
    {
        mTeamItems = other.mTeamItems; // deep copy since we're POD.
        mTotalNumItems = other.getTotalNumItems();
    }



    void teambacklogs::save_team_CSV(std::wostream &os, unsigned int teamNdx) const // output the backlog as a inputtable csv.
    {
        os << "Project,ID,Description,Min Calendar WorkDays,Remaining DevDays Total,Earliest Start,Blocking,Contributing,Depends On,Comments" << std::endl;
        for (auto &i : mTeamItems)
            for (auto & j : i)
                if (j.mTeamNdx == teamNdx)
                    j.output(os);
    }

    bool teambacklogs::exists(std::wstring id) const
    {
        for (const auto & i : mTeamItems)
            for (const auto & j : i)
                if (iSame(j.mId,id))
                    return true;
        return false;
    }

    const backlogitem & teambacklogs::getItemFromId(std::wstring id) const
    {
        ASSERT(mTeamItems.size()>0);
        for (const auto & i : mTeamItems)
            for (const auto & j : i)
                if (iSame(j.mId,id))
                    return j;
        TERMINATE(L"Failed to find backlog item with id "+id);
        return mTeamItems[0][0]; // code never gets here.
    }

    unsigned int teambacklogs::getTotalNumItems() const
    {
        return mTotalNumItems;
    }


} // namespace