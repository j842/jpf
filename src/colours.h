#ifndef __COLOURS_H
#define __COLOURS_H

#include <vector>
#include <string>

namespace colours
{


void turbo_getColour(const float value, float &red, float &green, float &blue);
void andrew_getColour(const float value, float &red, float &green, float &blue);

// Typical Use:

// ColorGradient heatMapGradient;    // Used to create a nice array of different colors.
// heatMapGradient.createDefaultHeatMapGradient();
// float r,g,b;
// heatMapGradient.getColorAtValue(yourGradientValue, r,g,b);

const std::string cWhite = "\e[38;5;253m";
const std::string cLightOrange = "\e[38;5;179m";
const std::string cBlue =  "\e[38;5;69m";
const std::string cLime = "\e[38;5;150m";
const std::string cNoColour = "\033[m";

const std::string cInfo = "\e[38;5;111m";
const std::string cDebug = "\e[38;5;239m";
const std::string cWarning = "\e[38;5;179m";
const std::string cError = "\e[38;5;9m";
const std::string cDefault = "\e[38;5;118m";
}

#endif
