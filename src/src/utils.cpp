#include <iostream>
#include <algorithm>
#include <array>
#include <memory>

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h> 
#include <sys/wait.h>
#include <filesystem>

#include "utils.h"
#include "settings.h"
#include "simplecsv.h"

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
    logerror(
        S() << "Terminating due to error:\n" << "   " << func << "\n   "<<file<<",  line "<<line
    );

    fatal( s );
}

void _terminate(const std::stringstream &s,const std::string &func, const std::string &file, int line)
{
    _terminate(s.str(),func,file,line);
}

void removewhitespace(std::string & s)
{
    s.erase( std::remove(s.begin(), s.end(), ' '), s.end());
}

// remove char if true.
bool codepredicate(char c)
{
    if (isalnum(c)) return false;
    std::string happy="_-";
    if (happy.find(c) != std::string::npos)
        return false;
    return true;
}

std::string makecode(const std::string & str) // alphanumeric, safe as HTML id etc.
{
    std::string s(str);
    s.erase(std::remove_if(s.begin(), s.end(), codepredicate), s.end());
    return s;
}

void trim(std::string &str)
{
    const char *typeOfWhitespaces = " \t\n\r\f\v";
    str.erase(str.find_last_not_of(typeOfWhitespaces) + 1);
    str.erase(0, str.find_first_not_of(typeOfWhitespaces));
}



watcher::watcher(std::vector<std::string> paths)
{
    //https://developer.ibm.com/tutorials/l-ubuntu-inotify/
    fd = inotify_init();
    if ( fd < 0 ) 
        TERMINATE( "inotify_init" );
    for (auto p : paths)
    {
        std::string pc = makecanonicalslash(p);
        if (std::filesystem::exists(pc))
            wd.push_back(
                inotify_add_watch( fd, pc.c_str(), IN_CLOSE_WRITE  | IN_CREATE | IN_DELETE )
            );
    }
}
watcher::~watcher()
{
    for (auto i : wd)
        inotify_rm_watch( fd, i );
    close( fd );    
}

void watcher::waitforchange() // create, modify, delete
{
    if (wd.size()>0)
        // block until one or more events arrive.
        /*length =*/ read( fd, buffer, BUF_LEN );  
}


void my_handler(int s){
    throw(ctrlcException("Exiting due to ctrl-c."));
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

    #pragma GCC diagnostic ignored "-Wignored-attributes"
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

std::string getDollars(double x)
{    
    long int kilodollarz = (int)(0.5 + x/1000.0);
    std::ostringstream oss;
    oss << kilodollarz*1000;

    std::string s = oss.str();
    unsigned int n = s.length();
    for (unsigned int i=n-1;i>0;i--)
    {
        unsigned int j = n-i;
        if (j%3==0)
            s.insert(s.begin()+i,',');
    }
    s.insert(s.begin(),'$');
    return s;
}


//---------------------------------------------------------------------------------------

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



listoutput::listoutput(std::ostream & ofs,std::string sstart, std::string seperator, std::string send) : 
    mOfs(ofs), mSeperator(seperator),mSEnd(send),mFirstItem(true),mEnded(false)
{
    mOfs << sstart;
}
listoutput::~listoutput()
{
    end(); 
}
void listoutput::write(std::string item) const
{
    if (!mFirstItem)
        mOfs << mSeperator;

    mOfs << item;
    mFirstItem = false;
}

void listoutput::writehq(std::string item) const // add halfquotes.
{
    listoutput::write(S() << "'" << item <<"'" );
}

void listoutput::end()
{
    if (mEnded)
        return;

     mOfs << mSEnd;
     mEnded=true;
}


void checkcreatedirectory(std::string d)
{
    if (!std::filesystem::exists(d))
    {
        if (!std::filesystem::create_directory(d))
            TERMINATE(S()<<"Could not create directory: "<<d);

        if (!std::filesystem::exists(d))
            TERMINATE(S()<<"Attempted to create directory, but it doesn't exist: "<<d);

        logdebug(S()<<"Created directory: "<<d);
    }    
}



std::string makelower(const std::string & s)
{
    std::string slower(s);
    std::transform(slower.begin(), slower.end(), slower.begin(),
        [](unsigned char c){ return std::tolower(c); });

    return slower;
}


unsigned long str2L(std::string s)
{
    unsigned long r=0;
    if (s.length()==0)
        return 0;
    try
    {
        r = stol(s);
    }
    catch (const std::invalid_argument & e)
    {
    }
    catch (const std::out_of_range & e)
    {
    }
    return r;
}
double str2positivedouble(std::string s)
{
    if (s.length()==0)
        return 0;
    double r = 0.0;
    try
    {
        r = stod(s);
    }
    catch (const std::invalid_argument & e)
    {
    }
    catch (const std::out_of_range & e)
    {
    }

    if (r<0.0)
    {
        logerror("String has negative number, but required to be positive by str2positivedouble.");
        return 0.0;
    }
    return r;    
}

void streamReplace(std::string ifile, std::ostream &ofs, const std::map<std::string,std::string> & replacerules)
{
    std::ifstream ifs(ifile);
    if (!ifs.is_open())
        fatal("Could not open file "+ifile);

    std::string s;
    while (std::getline(ifs, s))
    {
        replace(s,replacerules);
        ofs.write(s.c_str(),s.length());
        ofs<<std::endl;
    }
    ifs.close();

}


void streamReplace(std::string ifile, std::string ofile, const std::map<std::string,std::string> & replacerules)
{
    if (std::filesystem::exists(ofile))
        std::filesystem::remove(ofile);

    std::ofstream ofs(ofile);
    if (!ofs.is_open())
        fatal("Could not write to file "+ofile);

    streamReplace(ifile,ofs,replacerules);

    ofs.close();
}

void replace(std::string & s,  const std::map<std::string,std::string> & replacerules)
{
    for (auto iter = replacerules.begin(); iter != replacerules.end(); ++iter)
        s = replacestring(s,iter->first,iter->second); 
}


std::string replacestring(std::string subject, const std::string &search,
                          const std::string &replace)
{
    size_t pos = 0;
    if (search.empty() || subject.empty())
        return "";
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

std::string makecanonicalslash(const std::string &s)
{
    std::string ss(s);
    if (std::filesystem::exists(ss))
        ss = std::filesystem::canonical(ss);
    if (ss.back()!='/')
        ss.push_back('/');
    return ss;
}

void recreate_Directory(std::string path) 
{
    if (std::filesystem::exists(path))
        std::filesystem::remove_all(path);
    if (!std::filesystem::create_directory(path))
        TERMINATE(S() << "Could not create directory: " << path);
    logdebug(S() << "Created directory: " << path);
}




///-------------------------
/*
Code below here is copyrighted:

Copyright © Volker Diels-Grabsch
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


/* Escape a single character for LaTeX.
 *      returns a string constant (not to be freed)
 *      containing the character's replacement,
 *      or NULL if the character doesn't need to be replaced.
 */
static const char *escape_latex_char(char c)
{
    switch (c) {
        case '$':  return "\\$";
        case '%':  return "\\%";
        case '&':  return "\\&";
        case '#':  return "\\#";
        case '_':  return "\\_";
        case '{':  return "\\{";
        case '}':  return "\\}";
        case '[':  return "{[}";
        case ']':  return "{]}";
        case '"':  return "{''}";
        case '\\': return "\\textbackslash{}";
        case '~':  return "\\textasciitilde{}";
        case '<':  return "\\textless{}";
        case '>':  return "\\textgreater{}";
        case '^':  return "\\textasciicircum{}";
        case '`':  return "{}`"; /* avoid ?` and !` */
        case '\n': return "\\\\";
        default:   return NULL;
    }
}

/*! Escape a string for direct use in LaTeX.
 */
std::string latexesc(const std::string & istr)
{
    std::ostringstream oss;

    /* calculate result length */
    for (unsigned int i = 0; i<istr.length(); i++) {
        const char *escaped_char = escape_latex_char(istr[i]);
        if (escaped_char == NULL) {
            oss << istr[i];
        } else {
            oss << escaped_char;
        }
    }
    return oss.str();
}
