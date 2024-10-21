#include <climits>
#include <strings.h>

#include "inputfiles_projects.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

namespace inputfiles
{

    project::project(std::string id, simpledate targetd, std::string status, std::string name, std::string desc, bool BAU, const cTags & tags) : 
        mId(id), mTargetDate(targetd), mStatus(status), mName(name), mDescription(desc), mBAU(BAU), mTags(tags)
    {
    }

    bool isBAU(std::string s)
    {
        if (s.length() == 0)
        {
            logwarning("Empty string passed to isBAU. Assuming business as usual.");
            return true;
        }
        return (tolower(s[0]) == 'b');
    }

    projects::projects() : mMaxProjectNameWidth(0)
    {
        load_projects();
    }

    void projects::load_projects()
    {
        mMaxProjectNameWidth = 0;
        simplecsv c("projects.csv",6);

        if (!c.openedOkay())
            TERMINATE("Could not open projects.csv!");

        std::vector<std::string> row;
        while (c.getline(row, 7))
        {
            simpledate sd;
            if (row[1].length()==0) sd.setForever(); 
            else sd = simpledate(row[1]);

            project p(row[0], sd, row[2], row[3], row[4], isBAU(row[5]), cTags(row[6]));

            mMaxProjectNameWidth = std::max(mMaxProjectNameWidth, (unsigned int)p.getId().length());
            this->push_back(p);
        }
    }

    void projects::save_projects_CSV(std::ostream &os) const
    {
        os << R"(Project ID,TargetDate,Status,Project Name,Description,BAU or New,Tags)" << std::endl;

        for (const auto &i : *this)
        {
            std::vector<std::string> row = {
                i.getId(),
                i.getTargetDate().getStr(),
                i.getStatus(),
                i.getName(),
                i.getDesc(),
                i.getBAU() ? "BAU" : "New",
                i.getTags().getAsString(),
                };
            simplecsv::output(os, row);
            os << std::endl;
        }
    }

    unsigned int projects::getIndexByID(std::string id) const // returns eNotFound if index isn't there.
    {
        if (id.length()==0)
            return eNotFound;
        for (unsigned int i = 0; i < this->size(); ++i)
            if (iSame(this->at(i).getId(), id) || iSame(this->at(i).getName(),id))
                return i;
        return eNotFound;
    }

    void projects::debug_displayProjects() const
    {
        loginfo(S()<<"Loaded " << this->size() << " projects:");
        for (unsigned int i = 0; i < this->size(); ++i)
            loginfo(S() << "   " << i + 1 << "  " << this->at(i).getId() );
    }

    unsigned int projects::getMaxProjectNameWidth() const
    {
        return mMaxProjectNameWidth;
    }

    void projects::advance(workdate newStart)
    {
        // nothing to change in the projects file.
    }

} // namespace