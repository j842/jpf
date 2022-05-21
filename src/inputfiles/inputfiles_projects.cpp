#include <climits>
#include <strings.h>

#include "inputfiles_projects.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

namespace inputfiles
{

    project::project(std::wstring id, std::wstring name, std::wstring desc, bool BAU, std::wstring comments) : mId(id), mName(name), mDescription(desc), mBAU(BAU), mComments(comments)
    {
    }

    bool isBAU(std::wstring s)
    {
        if (s.length() == 0)
        {
            logwarning("Empty string passed to isBAU.");
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

        std::vector<std::wstring> row;
        while (c.getline(row, 5))
        {
            project p(row[0], row[1], row[2], isBAU(row[3]), row[4]);
            mMaxProjectNameWidth = std::max(mMaxProjectNameWidth, (unsigned int)p.getId().length());
            this->push_back(p);
        }
    }
    void projects::save_projects_CSV(std::ostream &os) const
    {
        os << R"(Project ID,Description,BAU or New,Comments)" << std::endl;

        for (const auto &i : *this)
        {
            std::vector<std::wstring> row = {
                i.getId(),
                i.getName(),
                i.getDesc(),
                i.getBAU() ? "BAU" : "New",
                i.getmComments()};
            simplecsv::output(os, row);
            os << std::endl;
        }
    }

    unsigned int projects::getIndexByID(std::wstring id) const // returns eNotFound if index isn't there.
    {
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