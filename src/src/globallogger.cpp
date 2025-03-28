#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include "settings.h"
#include "globallogger.h"
#include "colours.h"
#include "utils.h"

// LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE 
#ifdef LOGTOFILE
class FileStreamer
{
public:
    FileStreamer() : mLogFailedMsgSent(false)
    {
    }
    ~FileStreamer()
    {
    }

    bool CheckLogFileOpen(std::string mainlogfile)
    {
            if (!mCurrentStream.is_open())
                mCurrentStream.open(mainlogfile, std::ios_base::app);
            if (!mCurrentStream.is_open())
            {
                if (!mLogFailedMsgSent)
                {
                    mLogFailedMsgSent = true;
                    logmsg(kLWARN, "Logging suspended - could not open log file: " + mainlogfile);
                }
                return false;
            }

            if (mLogFailedMsgSent)
            {
                mLogFailedMsgSent = false;
                logmsg(kLINFO, "Now logging to log file: " + mainlogfile);
            }

        return true;
    }

    bool PastLimit(std::string s)
    {
        return static_cast<std::string::size_type>(mCurrentStream.tellp()) + s.length() > mLimit;
    }

    void Close()
    {
        mCurrentStream.close();
    }

    void Append(std::string s)
    {
        mCurrentStream << s;
    }

private:
    std::ofstream mCurrentStream;
    bool mLogFailedMsgSent;
    const size_t mLimit = 1024 * 1024 * 5;
};

FileStreamer g_FileStreamer;

void FileRotationLogSink(std::string s)
{
    static bool initialised=false;
    static std::string savedLog;
    if (gSettings().getRoot().length()==0)
    {
        savedLog+=s;
        return; // can't log to file until loaded.
    }
    if (!initialised)
    {
        if (!std::filesystem::exists(gSettings().getRoot()))
            return; // getting created no doubt...
        checkcreatedirectory(getOutputPath_Base()); // create path to log to.
        checkcreatedirectory(getOutputPath_Log()); // create path to log to.
        initialised=true;
    }
    if (savedLog.length()>0)
    {
        std::string sl=savedLog;
        savedLog.clear();
        FileRotationLogSink(savedLog);
    }

    std::string mainlogfile = getOutputPath_Log()+"jpf.log"; //"/var/log/jpf/jpf.log";

    if (!g_FileStreamer.CheckLogFileOpen(mainlogfile))
        return; // can't log to file.

    if (g_FileStreamer.PastLimit(s))
    {
        g_FileStreamer.Close();

        boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
        std::string archive_base =  getOutputPath_Log()+"jpf_" + simpledate(current_date).getStr_FileName();

        unsigned int i = 1;
        std::string archive;
        while (true)
        {
            archive = (S() << archive_base << "-" << i << ".log");
            if (std::filesystem::exists(archive))
                break;
            ++i;
        }

        if (0 != std::rename(mainlogfile.c_str(), archive.c_str()))
        {
            std::cerr << "Could not archive log file from" << std::endl
                      << mainlogfile << std::endl
                      << "to" << std::endl
                      << archive << std::endl;
        }

        // open new logfile
        if (!g_FileStreamer.CheckLogFileOpen(mainlogfile))
            return; // can't log to file.
    }

    g_FileStreamer.Append(s);
}
#endif
// LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE LOGTOFILE 



eLogLevel getMinLevel()
{
    return gSettings().getMinLogLevel();
}

std::string levelname(eLogLevel level)
{
    switch (level)
    {
    case kLDEBUG:
        return "DEBUG";
    case kLINFO:
        return "INFO ";
    case kLWARN:
        return "WARN ";
    case kLERROR:
        return "ERROR";
    default:
        logerror("Undefined log level specified!");
        return " ??? ";
    }
}

void logverbatim(eLogLevel level, std::string s)
{
    #ifdef LOGTOFILE
        FileRotationLogSink(s);
    #endif

    if (level < getMinLevel())
        return;

    // we use stdout for normal messages, stderr for warn and error.
    switch (level)
    {
    case kLDEBUG:
        std::cout << colours::cDebug << s << colours::cNoColour << std::flush;
        break;
    case kLINFO:
        std::cout << colours::cInfo << s << colours::cNoColour << std::flush;
        break;

    case kLWARN:
        std::cerr << colours::cWarning << s << colours::cNoColour << std::flush;
        break;

    case kLERROR:
        std::cerr << colours::cError << s << colours::cNoColour << std::flush;
        break;

    default:
        std::cerr << colours::cDefault << s << colours::cNoColour << std::flush;
        break;
    }
}

std::string getheader(eLogLevel level)
{
    std::ostringstream ost;
    time_t now = time(0);
    struct tm *timeinfo;
    char date_time[80];
    timeinfo = localtime(&now);

    strftime(date_time, 80, "%x %R", timeinfo);

    // char* date_time = ctime(&now);
    //  if (strlen(date_time)>0)
    //      date_time[strlen(date_time)-1]=0;

    ost << "|" << levelname(level) << "|" << date_time << "|  ";
    return ost.str();
}


void logmsg(eLogLevel level, std::string s)
{
    if (level < getMinLevel())
        return;

    std::string info = getheader(level);
    std::string s2 = replacestring(s, "\n", "\n" + info);
    // s2.erase(std::remove(s2.begin(), s2.end(), '\r'), s2.end());

    logverbatim(level, info + s2 + "\n");
}

int fatal(std::string s)
{
    throw TerminateRunException(s);
    return -1;
}

void logdebug(std::string s) { logmsg(kLDEBUG, (s)); }
void logerror(std::string s) { logmsg(kLERROR, (s)); }
void loginfo(std::string s) { logmsg(kLINFO, (s)); }
void logwarning(std::string s) { logmsg(kLWARN, (s)); }

// ----------------------------------------------------------------------------------------------------