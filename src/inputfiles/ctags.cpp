#include "ctags.h"
#include "../utils.h"

cTags::cTags() {}
cTags::cTags(std::string s)
{
    simplecsv::splitcsv(s, *this);
}
cTags::cTags(std::vector<std::string> tags) { *this = tags; }

std::string cTags::getAsString() const
{
    std::string r;
    for (auto &i : *this)
        r += (r.length() > 0 ? std::string(", ") + i : i);
    return r;
}

void cTags::addToTags(std::vector<std::string> &tags) const // add the stored tags to tags.
{
    for (auto &t : *this)
    {
        bool alreadyThere = false;
        for (auto &tt : tags)
            if (iSame(t, tt))
                alreadyThere = true;

        if (!alreadyThere)
            tags.push_back(makelower(t));
    }
}

bool cTags::hasTag(std::string tag) const
{
    for (auto &t : *this)
        if (iSame(t, tag))
            return true;
    return false;
}
