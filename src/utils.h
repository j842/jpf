#ifndef __UTILS_H
#define __UTILS_H

#include <climits>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/inotify.h>
#include <chrono>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

// a centiday is 1/100th of a day.
typedef int tCentiDay;

// string manipulation etc.
void removewhitespace(std::string & s);
void checkcreatedirectory(std::string d);
std::string makelower(const std::string & s);
bool iSame(const std::string &s1, const std::string &s2);
void trim(std::string & str);
std::string trimCSVentry(const std::string str);

// a few globals
const unsigned int eNotFound = UINT_MAX;


// termiantion routines.
void _terminate(const std::string &s, const std::string &func, const std::string &file, int line);
void _terminate(const std::stringstream &s, const std::string &func, const std::string &file, int line);
#define STR(x) #x
#define ASSERT(x) if (!(x)) { printf("assertion failed: (%s), function %s, file %s, line %d.\n", STR(x), __PRETTY_FUNCTION__, __FILE__, __LINE__); exit(1); }
#define TERMINATE(x) { _terminate(x, __PRETTY_FUNCTION__, __FILE__, __LINE__); }


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
        watcher(std::string path);
        ~watcher();

        void waitforchange(); // create, modify, delete

    private:
        int wd,fd;
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

#endif
