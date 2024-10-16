#include "args.h"
#include "utils.h"
#include "inputfiles/simplecsv.h"
#include "globallogger.h"

cArgs::cArgs(int argc, char **argv)
{
    if (argc>0)
        for (unsigned int i = 1; i < static_cast<unsigned int>(argc); ++i)
            _parse(argv[i]);
}

bool cArgs::_hasOpt(const std::string & s) const
{
    for (auto &opt : mOpts)
        if (iSame(opt.opt, s))
            return true;

    return false;
}

bool cArgs::hasOpt(const std::vector<std::string> & vs) const
{
    for (auto & s : vs)
        if (_hasOpt(s)) return true;
    return false;
}

std::string cArgs::getValue(const std::vector<std::string> & vs) const
{
    for (auto & s : vs)
        for (auto & opt : mOpts)
            if (iSame(opt.opt,s))
                return opt.value;
    return "";
}

size_t cArgs::numArgs() const
{
    return mArgs.size();
}

std::string cArgs::getArg(size_t ndx) const
{
    ASSERT(ndx < mArgs.size());
    return mArgs.at(ndx);
}

void cArgs::_parse(std::string a)
{
    if (a.length() == 0)
        return;
    trim(a);
    // dequote.
    if (a.length() > 1 && a[0] == '"' && a.back() == '"')
    {
        a.erase(0, 1);
        a.pop_back();
    }

    if (a.length() == 0)
        return;

    if (a[0] == '-')
    {
        const char *dashes = "-";
        a.erase(0, a.find_first_not_of(dashes));

        std::size_t epos = a.find('=');
        if (epos == 0)
            TERMINATE("Bad argument, starts with = sign: " + a);
        if (epos == std::string::npos)
            mOpts.push_back({.opt = a, .value = ""});
        else
            mOpts.push_back({.opt = a.substr(0, epos),
                             .value = a.substr(epos + 1)});
    }
    else
        mArgs.push_back(a);
}

bool cArgs::validate(const std::vector<std::string> & validOpts) const 
{
    bool rval = true;
    for (auto & i : mOpts)
        {
            bool valid = false;
            for (auto & j : validOpts)
                if (iSame(i.opt,j))
                    valid=true;
            if (!valid)
            {
                logerror("Invalid option: -"+i.opt);
                rval = false;
            }
        }   
    return rval; 
}
