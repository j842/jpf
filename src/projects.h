#ifndef __PROJECTS_H
#define __PROJECTS_H

#include <string>
#include <vector>

#include "itemdate.h"


class project
{
    public: 
        project(std::string id, std::string desc, bool BAU, std::string comments);

        const std::string & getId() const {return mId;}
        const std::string & getDesc() const {return mDescription;}
        bool getBAU() const {return mBAU;}
        const std::string & getmComments() const {return mComments;}

        // set when project scheduled.
        itemdate mActualStart;
        itemdate mActualEnd;
        double mTotalDevDays; // proportional to cost.

    private:
        const std::string mId;
        const std::string mDescription;
        const bool mBAU;
        const std::string mComments;
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
    
        void advance(itemdate newStart);

    private:
        unsigned int mMaxProjectNameWidth;
};

#endif