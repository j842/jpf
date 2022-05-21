#ifndef __PROJECTS_H
#define __PROJECTS_H

#include <string>
#include <vector>

#include "workdate.h"

namespace inputfiles
{

class project
{
    public: 
        project(std::wstring id, std::wstring name, std::wstring desc, bool BAU, std::wstring comments);

        const std::wstring & getId() const {return mId;}
        const std::wstring & getName() const {return mName;}
        const std::wstring & getDesc() const {return mDescription;}
        bool getBAU() const {return mBAU;}
        const std::wstring & getmComments() const {return mComments;}

    private:
        const std::wstring mId;
        const std::wstring mName;
        const std::wstring mDescription;
        const bool mBAU;
        const std::wstring mComments;
};

class projects : public std::vector<project>
{
    public:
        projects();
        void load_projects();
        void save_projects_CSV(std::wostream & os) const;

        unsigned int getMaxProjectNameWidth() const;
        void debug_displayProjects() const;

        unsigned int getIndexByID(std::wstring id) const; // returns eNotFound if index isn't there.
    
        void advance(workdate newStart);

    private:
        unsigned int mMaxProjectNameWidth;
};

} //namespace

#endif