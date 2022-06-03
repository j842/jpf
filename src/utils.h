#ifndef __UTILS_H
#define __UTILS_H

#include <climits>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/inotify.h>
#include <chrono>
#include <map>
#include <vector>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

class workdate; // forward declaration to avoid circular dependency.


// string manipulation etc.
void removewhitespace(std::string & s);
void checkcreatedirectory(std::string d);
std::string makelower(const std::string & s);
bool iSame(const std::string &s1, const std::string &s2);
void trim(std::string & str);
unsigned long str2L(std::string s);
double str2positivedouble(std::string s);
void advanceLeaveString(std::string & leaveStr, workdate newStart);
std::string getDollars(double x);

// a few globals
const unsigned int eNotFound = UINT_MAX;


// termiantion routines.
void _terminate(const std::string &s, const std::string &func, const std::string &file, int line);
void _terminate(const std::stringstream &s, const std::string &func, const std::string &file, int line);
#define STR(x) #x
#define ASSERT(x) if (!(x)) {  _terminate(S()<<"Assertation failed: "<<STR(x), __PRETTY_FUNCTION__, __FILE__, __LINE__); }
#define TERMINATE(x) { _terminate(x, __PRETTY_FUNCTION__, __FILE__, __LINE__); }

#define HALFWIDTHPADLEFT(n1,maxw)   (n1<maxw ? (int)(0.5 + 0.5*(maxw+n1)) : 0)
#define HALFWIDTHPADRIGHT(n1,maxw)  (n1<maxw ? maxw-HALFWIDTHPADLEFT(n1,maxw) : 0)
#define EXTRAPADRIGHT(n1,maxw)      (n1<maxw ? maxw-n1 : 0)
#define CENTERSTREAM(s,maxw)        std::setw(HALFWIDTHPADLEFT(s.length(),maxw)) <<  s  << std::setw(HALFWIDTHPADRIGHT(s.length(),maxw)) << ""
#define LEFTSTREAM(s,maxw)          s << std::setw(EXTRAPADRIGHT(s.length(),maxw)) << ""
#define RIGHTSTREAM(s,maxw)         std::setw(maxw) << s

// terminate current calculation
struct TerminateRunException : public std::exception
{
   std::string s;
   TerminateRunException(std::string ss) : s(ss) {}
   ~TerminateRunException() throw () {} // Updated
   const char* what() const throw() { return s.c_str(); }
};
//exit entire application
struct ctrlcException : public std::exception
{
   std::string s;
   ctrlcException(std::string ss) : s(ss) {}
   ~ctrlcException() throw () {} // Updated
   const char* what() const throw() { return s.c_str(); }
};



// class to inline ostringstream, which allows embedded end of line.
class S
{
private:
    std::ostringstream os_stream;

public:
    S() {}
    S(const char *s) : os_stream(s) {}
    S(const std::string &s) : os_stream(s) {}
    template <class T>
    S &operator<<(const T &t)
    {
        os_stream << t;
        return *this;
    }
    S &operator<<(std::ostream &(*f)(std::ostream &))
    {
        os_stream << f;
        return *this;
    }
    operator std::string() const
    {
        return os_stream.str();
    }
};


// watch a folder for changes in it.
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
class watcher
{
    public:
        watcher(std::vector<std::string> paths);
        ~watcher();

        void waitforchange(); // create, modify, delete

    private:
        std::vector<int> wd;
        int fd;
        char buffer[BUF_LEN];
};
    
// routine to trap ctrl-c.
void catch_ctrl_c();

class webserver
{
    public:
        webserver(int port);
        ~webserver();

    private:
        pid_t mChildPid;
};


class timer
{
    public:
        timer();
        double stop();
        double getms();
    private:
        std::chrono::_V2::system_clock::time_point t0;
        double ms;
};

// output a list of things to a stream.
class listoutput
{
    public:
        listoutput(std::ostream & ofs,std::string sstart, std::string seperator, std::string send);
        ~listoutput();
        void write(std::string item) const;
        void writehq(std::string item) const; // add halfquotes.
        void end();

    private:
        std::ostream & mOfs;
        std::string mSeperator;
        std::string mSEnd;
        mutable bool mFirstItem;
        bool mEnded;
};

void streamReplace(std::string ifile, std::string ofile, const std::map<std::string,std::string> & replacerules);
void streamReplace(std::string ifile, std::ostream &ofs, const std::map<std::string,std::string> & replacerules);
void replace(std::string & s,  const std::map<std::string,std::string> & replacerules);
std::string replacestring(std::string subject, const std::string &search, const std::string &replace);
std::string makecanonicalslash(const std::string & s);

#endif
