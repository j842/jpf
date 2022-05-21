#ifndef __gantt__H
#define __gantt__H

#include <string>

class gantt
{
    public:
        static bool output(std::string path); // recreate output files. Path needs to end in /

    private:
   static bool output_frappe_gantt_css(std::string path);
   static bool output_frappe_gantt_js(std::string path);
   static bool output_frappe_gantt_js_map(std::string path);
   static bool output_frappe_gantt_min_css(std::string path);
   static bool output_frappe_gantt_min_js(std::string path);
   static bool output_frappe_gantt_min_js_map(std::string path);
   static bool output_test_html(std::string path);
};

#endif
