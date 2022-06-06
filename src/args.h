#ifndef __ARGS_H
#define __ARGS_H

#include <string>
#include <vector>

// -x=value, --x=value, --xibble=value
struct CArgOpt
{
    std::string opt;
    std::string value;
};


class cArgs
{
    public:
        cArgs(int argc, char ** argv);

        bool validate(const std::vector<std::string> & validOpts) const;

        bool hasOpt(const std::vector<std::string> & vs) const;
        std::string getValue(const std::vector<std::string> & vs) const;

        size_t numArgs() const;
        std::string getArg(size_t ndx) const;

    private:
        void _parse(std::string a);
        bool _hasOpt(const std::string & s) const;

    private:
        std::vector<CArgOpt> mOpts;
        std::vector<std::string> mArgs;
};

#endif