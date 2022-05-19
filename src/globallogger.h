#ifndef __GLOBALLOGGER_H
#define __GLOBALLOGGER_H

#include <string>
#include <memory>

enum eLogLevel
{
   kLDEBUG=0,
   kLINFO=1,
   kLWARN=2,
   kLERROR=3
};

void logmsg(eLogLevel level, std::string s);
void logverbatim(eLogLevel level, std::string s);

void logdbg(std::string s);
void logdbg_trim(std::string s);
void logerror(std::string s);
void fatal(std::string s);

#endif
