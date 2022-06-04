#ifndef __CTAGS_H
#define __CTAGS_H

#include "simplecsv.h"

    class cTags : public std::vector<std::string> 
    {
        public:
            cTags(); 
            cTags(std::string s);
            cTags(std::vector<std::string> tags);

            std::string getAsString() const;

            void addToTags(std::vector<std::string> & tags) const; // add the stored tags to tags.
            bool hasTag(std::string tag) const;
    };


#endif