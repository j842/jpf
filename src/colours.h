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

    extern const colourcode cWhite;
    extern const colourcode cLightOrange;
    extern const colourcode cBlue;
    extern const colourcode cLime;
    extern const colourcode cNoColour;

    extern const colourcode cInfo;
    extern const colourcode cDebug;
    extern const colourcode cWarning;
    extern const colourcode cError;
    extern const colourcode cDefault;
}

#endif
