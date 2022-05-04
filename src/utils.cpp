#include <iostream>
#include <algorithm>
#include <array>
#include <memory>

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h> 

#include "utils.h"
#include "settings.h"

bool iSame(const std::string &s1, const std::string &s2)
{
    if (s1.size()!=s2.size())
        return false;
    for (unsigned int i=0;i<s1.size();++i)
        if (tolower(s1[i])!=tolower(s2[i]))
            return false;

    return true;
}


void _terminate(const std::string &s,const std::string &func, const std::string &file, int line)
{
    std::string ss;
    {
        std::ostringstream oss;

        oss << std::endl << std::endl;
        oss << "Terminating due to error (" << func << ",  "<<file<<",  line "<<line<< ") : "<<std::endl<<std::endl;
        oss << s << std::endl << std::endl;

        ss = oss.str();
    }

    throw TerminateRunException(ss);
}

void _terminate(const std::stringstream &s,const std::string &func, const std::string &file, int line)
{
    _terminate(s.str(),func,file,line);
}

void removewhitespace(std::string & s)
{
    s.erase( std::remove(s.begin(), s.end(), ' '), s.end());
}



watcher::watcher(std::string path)
{
    //https://developer.ibm.com/tutorials/l-ubuntu-inotify/
    fd = inotify_init();
    if ( fd < 0 ) 
        TERMINATE( "inotify_init" );
    wd = inotify_add_watch( fd, path.c_str(), IN_CLOSE_WRITE  | IN_CREATE | IN_DELETE ); // IN_MODIFY gets called 2x...
}
watcher::~watcher()
{
    inotify_rm_watch( fd, wd );
    close( fd );    
}

void watcher::waitforchange() // create, modify, delete
{
    // block until one or more events arrive.
    /*length =*/ read( fd, buffer, BUF_LEN );  
}


void my_handler(int s){
    throw(ctrlcException("Exiting due to ctrl-c."));
    exit(1); 
}

void catch_ctrl_c()
{
    struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = my_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);
}

std::string customexec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        TERMINATE("Couldn't open pipe to external command.");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string starbox(std::string s)
{
    std::string ss(s.length()+4,'*');
    return S() << ss << std::endl << "* "+s+" *" <<std::endl << ss;
}

webserver::webserver(int port)
{
    const std::string webfsd("webfsd-jpf");
    std::ostringstream portstr;
    portstr << port;
    fflush(NULL);
    pid_t pid = fork();
    if (pid > 0) {
        return;
    } 
    else if (pid == 0) 
    {
        std::cout <<std::endl<< "Starting webserver for "<<getOutputPath_Html()<<", listening to port "<<port<<"." << std::endl<<std::endl;
        execlp(webfsd.c_str(), webfsd.c_str(), "-p",portstr.str().c_str(),"-r",getOutputPath_Html().c_str(),"-f","index.html","-n","localhost","-F", NULL);
        // if we are here execl has failed 
        std::cerr << starbox("Unable to start webserver. Please check webfsd is installed and not already running.") << std::endl;
        exit(1);
    }
    else
        TERMINATE("Could not create child process.");
}

webserver::~webserver()
{
}

timer::timer() : t0( std::chrono::high_resolution_clock::now()  )
{
}

double timer::stop()
{
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_double = t2-t0;
    ms=ms_double.count();

    return ms;
}

double timer::getms()
{
    return ms;
}