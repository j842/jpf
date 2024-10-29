#include <climits>
#include <strings.h>

#include "inputfiles_projects.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

namespace inputfiles
{

    project::project(std::string id, simpledate targetd, std::string status, std::string name, std::string desc, 
                        eProjectType type, const cTags & tags, simpledate approvedd) : 
        mId(id), mTargetDate(targetd), mStatus(status), mName(name), mDescription(desc), 
        mType(type), mTags(tags), mApprovedDate(approvedd)
    {
    }

    projects::projects() : mMaxProjectNameWidth(0)
    {
        load_projects();
    }

    simpledate str2date_blankIsForever(const std::string & s)
    {
        simpledate sd;
        if (s.length()==0) sd.setForever();
        else sd = simpledate(s);
        return sd;
    }

    void projects::load_projects()
    {
        mMaxProjectNameWidth = 0;
        simplecsv c("projects.csv",6);

        if (!c.openedOkay())
            TERMINATE("Could not open projects.csv!");

        std::vector<std::string> row;
        while (c.getline(row, 8))
        {
            simpledate sd = str2date_blankIsForever(row[1]);
            simpledate ad = str2date_blankIsForever(row[7]);

            project p(row[0], sd, row[2], row[3], row[4], ProjectTypefromString(row[5]), cTags(row[6]), ad);

            mMaxProjectNameWidth = std::max(mMaxProjectNameWidth, (unsigned int)p.getId().length());
            this->push_back(p);
        }
    }

    void projects::save_projects_CSV(std::ostream &os) const
    {
        os << R"(Project ID,TargetDate,Status,Project Name,Description,Type,Tags,ApprovedDate)" << std::endl;

        for (const auto &i : *this)
        {
            std::vector<std::string> row = {
                i.getId(),
                i.getTargetDate().getStr(),
                i.getStatus(),
                i.getName(),
                i.getDesc(),
                ProjectTypetoString(i.getType()),
                i.getTags().getAsString(),
                i.getApprovedDate().getStr()
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

std::string ProjectTypetoString(eProjectType t)
{
    switch (t)
    {
        case kPTBug: 
            return "Bug";
        case kPTBAU:
            return "BAU";
        case kPTNew:
            return "New";
        default:
            ;
    }
    return ""; // kNone
}

eProjectType ProjectTypefromString(const std::string &s)
{
    if (iSame(s,"bug")) return kPTBug;
    if (iSame(s,"BAU")) return kPTBAU;
    if (iSame(s,"New")) return kPTNew;

    logwarning(S()<<"Invalid Project Type: "<<s);
    return kPTNone;
}
