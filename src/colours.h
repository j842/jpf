#ifndef __COLOURS_H
#define __COLOURS_H

#include <vector>
#include <string>

namespace colours
{

class ColorGradient
{
private:
  struct ColorPoint  // Internal class used to store colors at different points in the gradient.
  {
    float r,g,b;      // Red, green and blue values of our color.
    float val;        // Position of our color along the gradient (between 0 and 1).
    ColorPoint(float red, float green, float blue, float value);
  };
  std::vector<ColorPoint> color;      // An array of color points in ascending value.
  
public:
  ColorGradient();
  void addColorPoint(float red, float green, float blue, float value);
  void clearGradient();
  void createDefaultHeatMapGradient();
  void getColorAtValue(const float value, float &red, float &green, float &blue);
};

// Typical Use:

// ColorGradient heatMapGradient;    // Used to create a nice array of different colors.
// heatMapGradient.createDefaultHeatMapGradient();
// float r,g,b;
// heatMapGradient.getColorAtValue(yourGradientValue, r,g,b);

const std::wstring cWhite = "\e[38;5;253m";
const std::wstring cLightOrange = "\e[38;5;179m";
const std::wstring cBlue =  "\e[38;5;69m";
const std::wstring cLime = "\e[38;5;150m";
const std::wstring cNoColour = "\033[m";

const std::wstring cInfo = "\e[38;5;111m";
const std::wstring cDebug = "\e[38;5;239m";
const std::wstring cWarning = "\e[38;5;179m";
const std::wstring cError = "\e[38;5;9m";
const std::wstring cDefault = "\e[38;5;118m";
}

#endif
