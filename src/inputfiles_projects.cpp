#include <climits>
#include <strings.h>

#include "inputfiles_projects.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

namespace inputfiles
{

    project::project(std::string id, std::string desc, bool BAU, std::string comments) : mId(id), mDescription(desc), mBAU(BAU), mComments(comments)
    {
    }

    bool isBAU(std::string s)
    {
        if (s.length() == 0)
        {
            std::cout << "ERROR - empty string passed to isBAU." << std::endl;
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
        simplecsv c("projects.csv");

        if (!c.openedOkay())
            TERMINATE("Could not open projects.csv!");

        std::vector<std::string> row;
        while (c.getline(row, 4))
        {
            project p(row[0], row[1], isBAU(row[2]), row[3]);
            mMaxProjectNameWidth = std::max(mMaxProjectNameWidth, (unsigned int)p.getId().length());
            this->push_back(p);
        }
    }
    void projects::save_projects_CSV(std::ostream &os) const
    {
        os << R"(Project ID,Description,BAU or New,Comments)" << std::endl;

        for (const auto &i : *this)
        {
            std::vector<std::string> row = {
                i.getId(),
                i.getDesc(),
                i.getBAU() ? "BAU" : "New",
                i.getmComments()};
            simplecsv::output(os, row);
            os << std::endl;
        }
    }

    unsigned int projects::getIndexByID(std::string id) const // returns eNotFound if index isn't there.
    {
        for (unsigned int i = 0; i < this->size(); ++i)
            if (iSame(this->at(i).getId(), id))
                return i;
        return eNotFound;
    }

    void projects::debug_displayProjects() const
    {
        std::cout << std::endl;
        std::cout << "Loaded " << this->size() << " projects:" << std::endl;
        for (unsigned int i = 0; i < this->size(); ++i)
            std::cout << "   " << i + 1 << "  " << this->at(i).getId() << std::endl;
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