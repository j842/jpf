#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "settings.h"
#include "globallogger.h"

class FileStreamer
{
public:
    FileStreamer() : mLogFailedMsgSent(false)
    {
        mMainlogFile = gSettings().getRoot() + "/output/log.txt";
    }
    ~FileStreamer()
    {
    }

    void Log(std::string s)
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
    std::string mMainlogFile;
    std::ofstream mCurrentStream;
    bool mLogFailedMsgSent;
    const size_t mLimit = 1024 * 1024 * 5;
};

FileStreamer g_FileStreamer;

void FileRotationLogSink(std::string s)
{
    std::string mainlogfile = gSettings().getRoot() + "/output/log.txt";

    if (!g_FileStreamer.CheckLogFileOpen(mainlogfile))
        return; // can't log to file.

    if (g_FileStreamer.PastLimit(s))
    {
        g_FileStreamer.Close();

        boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
        std::string archive = gSettings().getRoot()+"/output/"+simpledate(current_date).getStr() + "_log.txt";

        if (0 != std::rename(mainlogfile.c_str(), archive.c_str()))
        {
            std::cerr << "Could not archive log file from" << std::endl
                        << mainlogfile << std::endl
                        << "to" << std::endl
                        << archive << std::endl;
            exit(1);
        }

        // open new logfile
        if (!g_FileStreamer.CheckLogFileOpen(mainlogfile))
            return; // can't log to file.
    }

    g_FileStreamer.Append(s);
}

eLogLevel getMinLevel()
{
    return kLDEBUG;
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
        return " ??? ";
    }
}

void logverbatim(eLogLevel level, std::string s)
{
    if (level < getMinLevel())
        return;

    FileRotationLogSink(s);

    // we use stdout for normal messages, stderr for warn and error.
    switch (level)
    {
    case kLDEBUG:
        std::cout << termcolor::cyan << s << termcolor::reset;
        break;
    case kLINFO:
        std::cout << termcolor::blue << s << termcolor::reset;
        break;

#ifdef _WIN32
    case kLWARN:
        std::cerr << termcolor::red << s << termcolor::reset;
        break;
#else
    case kLWARN:
        std::cerr << termcolor::yellow << s << termcolor::reset;
        break;
#endif

    case kLERROR:
        std::cerr << termcolor::red << s << termcolor::reset;

#ifdef _DEBUG
#ifdef _WIN32
        __debugbreak();
#else
        __builtin_trap();
#endif
#endif

        throw eExit();
        break;
    default:
        std::cerr << termcolor::green << s << termcolor::reset;
        break;
    }
}

std::string getheader(eLogLevel level)
{
    std::ostringstream ost;
    ost << "|" << levelname(level) << "|" << timeutils::getLogTimeStamp() << "| ";
    return ost.str();
}

void logmsg(eLogLevel level, std::string s)
{
    if (level < getMinLevel())
        return;

    std::string info = getheader(level);
    std::string s2 = utils::replacestring(s, "\n", "\n" + info);
    // boost::erase_all(s2, "\r");
    s2.erase(std::remove(s2.begin(), s2.end(), '\r'), s2.end());

    logverbatim(level, info + s2 + "\n");
}

void logdbg(std::string s)
{
    logmsg(kLDEBUG, s);
}

void logdbg_trim(std::string s)
{
    Poco::trimInPlace(s);
    if (s.length() > 0)
        logdbg(s);
}

void fatal(std::string s)
{
    logmsg(kLERROR, s);
}

// ----------------------------------------------------------------------------------------------------