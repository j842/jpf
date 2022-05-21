// auto-generated cpp file.
// created from templates/header.html.

#include <fstream>
#include "header.html.h"

void header_html::replacestring(std::wstring & subject, const std::wstring &search, const std::wstring &replace)
{
    size_t pos = 0;
    if (search.empty() || subject.empty()) return;
    while ((pos = subject.find(search, pos)) != std::wstring::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

/*static*/ bool header_html::output(std::string path, const std::map<std::wstring,std::wstring> * replacerules)
{
   if (path.length()==0) return false;
   if (path[path.length()-1]!='/')
      path.push_back('/');
   std::wofstream os(path+getName());
   if (!os.is_open())
      return false;
   os << getStr(replacerules);
   os.close();
}

/*static*/ std::string header_html::getName()
{
   return "header.html";
}

/*static*/ std::wstring header_html::getStr(const std::map<std::wstring,std::wstring> * replacerules)
{
   std::wstring s= 
LR"LITERAL(<!----------------  HEADER ------------------------------->

<h1>^title^</h1>

<a href="index.html">Project Backlog</a>&nbsp;&nbsp;
<a href="people.html">People Backlog</a>&nbsp;&nbsp;
<a href="peopleeffort.html">People Effort</a>&nbsp;&nbsp;
<a href="costdashboard.html">Cost Dashboard</a>&nbsp;&nbsp;
<a href="highlevelgantt.html">High-Level Gantt</a>&nbsp;&nbsp;
<a href="detailedgantt.html">Detailed Gantt</a>&nbsp;&nbsp;
<a href="raw_backlog.html">Debug: Raw Backlog</a>&nbsp;&nbsp;
<p style="margin-bottom:1cm;"/>

<!---------------- /HEADER ------------------------------->
)LITERAL";
   if (replacerules!=NULL)
      {
         for (auto iter = replacerules->begin(); iter != replacerules->end(); ++iter)
            replacestring(s,iter->first,iter->second); 
      }
   return s;
}

// end of auto-generated cpp file.
