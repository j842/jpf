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

std::string levelname(eLogLevel level);

void logmsg(eLogLevel level, std::string s);
void logverbatim(eLogLevel level, std::string s);

void logdebug(std::string s);
void logerror(std::string s);
void loginfo(std::string s);
void logwarning(std::string s);
void fatal(std::string s);

#endif
