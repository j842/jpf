#ifndef __header_html__H
#define __header_html__H

#include <string>
#include <map>

class header_html
{
   public:
      static bool output(std::string path, const std::map<std::wstring,std::wstring> * replacerules = NULL); // recreate output file. 
      static std::wstring getStr(const std::map<std::wstring,std::wstring> * replacerules = NULL);  // contents of output file, with replacements as per rules.
      static std::string getName(); // output file name.

   private:
      static void replacestring(std::wstring & subject, const std::wstring &search, const std::wstring &replace);
};

#endif
