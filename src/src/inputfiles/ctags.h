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

    void copyinto(std::vector<std::string> &tags) const; // add the stored tags to tags.
    void mergefrom(const std::vector<std::string> &tags);
    bool hasTag(std::string tag) const;

    template <class T>
    void mergefromT(const std::vector<T> &tags)
    {
        for (const auto &t : tags)
            if (!hasTag(static_cast<std::string>(t)))
                this->push_back(static_cast<std::string>(t));
    }
};


class ciSet : public std::vector<unsigned int>
{
public:
    ciSet();
    void add(unsigned int x);
    void add(const ciSet & other);
    bool has(unsigned int x) const;
};


#endif