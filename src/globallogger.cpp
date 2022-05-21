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

    bool PastLimit(std::wstring s)
    {
        return static_cast<std::wstring::size_type>(mCurrentStream.tellp()) + s.length() > mLimit;
    }

    void Close()
    {
        mCurrentStream.close();
    }

    void Append(std::wstring s)
    {
        mCurrentStream << s;
    }

private:
    std::wofstream mCurrentStream;
    bool mLogFailedMsgSent;
    const size_t mLimit = 1024 * 1024 * 5;
};

FileStreamer g_FileStreamer;

void FileRotationLogSink(std::wstring s)
{
    static bool initialised=false;
    static std::wstring savedLog;
    if (gSettings().getRoot().length()==0)
    {
        savedLog+=s;
        return; // can't log to file until loaded.
    }
    if (!initialised)
    {
        checkcreatedirectory(getOutputPath_Base()); // create path to log to.
        checkcreatedirectory(getOutputPath_Log()); // create path to log to.
        initialised=true;
    }
    if (savedLog.length()>0)
    {
        std::wstring sl=savedLog;
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
            archive = w2s(S() << archive_base << "-" << i << ".log");
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

eLogLevel getMinLevel()
{
    return gSettings().getMinLogLevel();
}

std::wstring levelname(eLogLevel level)
{
    switch (level)
    {
    case kLDEBUG:
        return L"DEBUG";
    case kLINFO:
        return L"INFO ";
    case kLWARN:
        return L"WARN ";
    case kLERROR:
        return L"ERROR";
    default:
        logerror("Undefined log level specified!");
        return L" ??? ";
    }
}

void logverbatim(eLogLevel level, std::wstring s)
{
    FileRotationLogSink(s);

    if (level < getMinLevel())
        return;

    // we use stdout for normal messages, stderr for warn and error.
    switch (level)
    {
    case kLDEBUG:
        std::wcout << colours::cDebug << s << colours::cNoColour;
        break;
    case kLINFO:
        std::wcout << colours::cInfo << s << colours::cNoColour;
        break;

    case kLWARN:
        std::wcerr << colours::cWarning << s << colours::cNoColour;
        break;

    case kLERROR:
        std::wcerr << colours::cError << s << colours::cNoColour;
        break;

    default:
        std::wcerr << colours::cDefault << s << colours::cNoColour;
        break;
    }
}
void logverbatim(eLogLevel level, std::string s)
{
    logverbatim(level,s2w(s));
}

std::wstring getheader(eLogLevel level)
{
    std::wostringstream ost;
    time_t now = time(0);
    struct tm *timeinfo;
    char date_time[80];
    timeinfo = localtime(&now);

    strftime(date_time, 80, "%x %R", timeinfo);

    // char* date_time = ctime(&now);
    //  if (strlen(date_time)>0)
    //      date_time[strlen(date_time)-1]=0;

    ost << L"|" << levelname(level) << L"|" << date_time << L"|  ";
    return ost.str();
}


void logmsg(eLogLevel level, std::wstring s)
{
    if (level < getMinLevel())
        return;

    std::wstring info = getheader(level);
    std::wstring s2 = replacestring(s, L"\n", L"\n" + info);
    // s2.erase(std::remove(s2.begin(), s2.end(), '\r'), s2.end());

    logverbatim(level, info + s2 + L"\n");
}
void logmsg(eLogLevel level, std::string s)
{
    logmsg(level,s2w(s));
}

void fatal(std::wstring s)
{
    throw TerminateRunException(s);
}
void fatal(std::string s)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    throw TerminateRunException(converter.from_bytes(s));
}

std::wstring s2w(std::string s)
{
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(s);
}
std::string w2s(std::wstring s)
{
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(s);
}

void logdebug(std::wstring s) { logmsg(kLDEBUG, (s)); }
void logerror(std::wstring s) { logmsg(kLERROR, (s)); }
void loginfo(std::wstring s) { logmsg(kLINFO, (s)); }
void logwarning(std::wstring s) { logmsg(kLWARN, (s)); }
void logdebug(std::string s) { logmsg(kLDEBUG, s2w(s)); }
void logerror(std::string s) { logmsg(kLERROR, s2w(s)); }
void loginfo(std::string s) { logmsg(kLINFO, s2w(s)); }
void logwarning(std::string s) { logmsg(kLWARN, s2w(s)); }

// ----------------------------------------------------------------------------------------------------