#include <climits>
#include <strings.h>

#include "inputfiles_projects.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

namespace inputfiles
{

    project::project(std::string id, std::string name, std::string desc, bool BAU, std::string status, const cTags & tags, simpledate targetd) : 
        mId(id), mName(name), mDescription(desc), mBAU(BAU), mStatus(status), mTags(tags), mTargetDate(targetd)
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
            if (row[6].length()==0) sd.setForever(); 
            else sd = simpledate(row[6]);

            project p(row[0], row[1], row[2], isBAU(row[3]), row[4], cTags(row[5]), sd);

            mMaxProjectNameWidth = std::max(mMaxProjectNameWidth, (unsigned int)p.getId().length());
            this->push_back(p);
        }
    }

    void projects::save_projects_CSV(std::ostream &os) const
    {
        os << R"(Project ID,Project Name,Description,BAU or New,Status,Tags,TargetDate)" << std::endl;

        for (const auto &i : *this)
        {
            std::vector<std::string> row = {
                i.getId(),
                i.getName(),
                i.getDesc(),
                i.getBAU() ? "BAU" : "New",
                i.getStatus(),
                i.getTags().getAsString(),
                i.getTargetDate().getStr()
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