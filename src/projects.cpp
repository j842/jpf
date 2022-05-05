#include <climits>
#include <strings.h>

#include "projects.h"
#include "simplecsv.h"
#include "utils.h"
#include "settings.h"

project::project(std::string id, std::string desc, bool BAU, std::string comments) :
    mId(id), mDescription(desc), mBAU(BAU), mComments(comments)
    {}


projects::projects()
{
    simplecsv c("projects.csv");

    if (!c.openedOkay())
        TERMINATE("Could not open projects.csv!");

    std::vector<std::string> row;
    while (c.getline(row,4))
    {
        project p( row[0], row[1], iSame(row[2],"BAU"), row[3] );
        this->push_back(p);
    }
}

unsigned int projects::getIndexByID(std::string id) const // returns eNotFound if index isn't there.
{
    for (unsigned int i=0;i<this->size();++i)
        if (iSame(this->at(i).getId(), id))
            return i;
    return eNotFound;
}

void projects::debug_displayProjects() const
{
    std::cout << std::endl;
    std::cout << "Loaded "<< this->size() << " projects:" << std::endl;
    for (unsigned int i=0;i<this->size();++i)
        std::cout<<"   " << i+1 << "  " <<this->at(i).getId() << std::endl;
}