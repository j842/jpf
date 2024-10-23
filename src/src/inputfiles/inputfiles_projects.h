#ifndef __PROJECTS_H
#define __PROJECTS_H

#include <string>
#include <vector>

#include "workdate.h"
#include "ctags.h"

typedef enum eProjectType
{
    kPTNone,
    kPTBug,
    kPTBAU,
    kPTNew,
} eProjectType;

std::string ProjectTypetoString(eProjectType t);
eProjectType ProjectTypefromString(const std::string & s);

namespace inputfiles
{

class project
{
    public: 
        project(std::string id, simpledate targetd, std::string status, std::string name, std::string desc, 
                eProjectType type, const cTags & tags);

        const std::string & getId() const {return mId;}
        const simpledate getTargetDate() const {return mTargetDate;}
        const std::string & getStatus() const {return mStatus;}
        const std::string & getName() const {return mName;}
        const std::string & getDesc() const {return mDescription;}
        const eProjectType & getType() const {return mType;}
        const cTags & getTags() const {return mTags;}

    private:
        const std::string mId;
        const simpledate mTargetDate;
        const std::string mStatus;
        const std::string mName;
        const std::string mDescription;
        const eProjectType mType;
        const cTags mTags;
};

class projects : public std::vector<project>
{
    public:
        projects();
        void load_projects();
        void save_projects_CSV(std::ostream & os) const;

        unsigned int getMaxProjectNameWidth() const;
        void debug_displayProjects() const;

        unsigned int getIndexByID(std::string id) const; // returns eNotFound if index isn't there.
    
        void advance(workdate newStart);

    private:
        unsigned int mMaxProjectNameWidth;
};

} //namespace

#endif