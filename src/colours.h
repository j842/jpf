#ifndef __COLOURS_H
#define __COLOURS_H

#include <vector>
#include <string>
#include <iostream>

namespace colours
{
    void turbo_getColour(const float value, float &red, float &green, float &blue);
    void andrew_getColour(const float value, float &red, float &green, float &blue);

    class colourcode
    {
    public:
        colourcode(std::string console, std::string html);
        operator const std::string &() const { return _getstr(); }
        friend std::ostream &operator<<(std::ostream &os, const colourcode &cc);

    private:
        const std::string &_getstr() const;
        static std::string _cstart(std::string col);

        std::string console_code;
        std::string html_code;
    };



    const colourcode cWhite("\e[38;5;253m", "black");
    const colourcode cLightOrange("\e[38;5;179m", "orange");
    const colourcode cBlue("\e[38;5;69m", "blue");
    const colourcode cLime("\e[38;5;150m", "green");
    const colourcode cNoColour("\033[m", "</span>");

    const colourcode cInfo("\e[38;5;111m", "blue");
    const colourcode cDebug("\e[38;5;239m", "grey");
    const colourcode cWarning("\e[38;5;179m", "orange");
    const colourcode cError("\e[38;5;9m", "red");
    const colourcode cDefault("\e[38;5;118m", "black");
}

#endif
