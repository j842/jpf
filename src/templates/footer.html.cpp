// auto-generated cpp file.
// created from templates/footer.html.

#include <fstream>
#include "footer.html.h"

void footer_html::replacestring(std::wstring & subject, const std::wstring &search, const std::wstring &replace)
{
    size_t pos = 0;
    if (search.empty() || subject.empty()) return;
    while ((pos = subject.find(search, pos)) != std::wstring::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

/*static*/ bool footer_html::output(std::string path, const std::map<std::wstring,std::wstring> * replacerules)
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

/*static*/ std::string footer_html::getName()
{
   return "footer.html";
}

/*static*/ std::wstring footer_html::getStr(const std::map<std::wstring,std::wstring> * replacerules)
{
   std::wstring s= 
LR"LITERAL(<!----------------  FOOTER ------------------------------->

<p style="margin-bottom:1cm;"/>
<footer>
<font size="-1">
jpf version ^version^, files generated ^timedate^.
</font>
</footer>

<!---------------- /FOOTER ------------------------------->
)LITERAL";
   if (replacerules!=NULL)
      {
         for (auto iter = replacerules->begin(); iter != replacerules->end(); ++iter)
            replacestring(s,iter->first,iter->second); 
      }
   return s;
}

// end of auto-generated cpp file.
