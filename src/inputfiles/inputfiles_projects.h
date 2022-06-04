#ifndef __PROJECTS_H
#define __PROJECTS_H

#include <string>
#include <vector>

#include "workdate.h"
#include "ctags.h"

namespace inputfiles
{

class project
{
    public: 
        project(std::string id, std::string name, std::string desc, bool BAU, std::string comments, const cTags & tags);

        const std::string & getId() const {return mId;}
        const std::string & getName() const {return mName;}
        const std::string & getDesc() const {return mDescription;}
        bool getBAU() const {return mBAU;}
        const std::string & getmComments() const {return mComments;}
        const cTags & getTags() const {return mTags;}

    private:
        const std::string mId;
        const std::string mName;
        const std::string mDescription;
        const bool mBAU;
        const std::string mComments;
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