#include "colours.h"

using namespace std;

namespace colours
{

    // https://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients

    ColorGradient::ColorPoint::ColorPoint(float red, float green, float blue, float value)
        : r(red), g(green), b(blue), val(value) {}

    ColorGradient::ColorGradient() { createDefaultHeatMapGradient(); }

    //-- Inserts a new color point into its correct position:
    void ColorGradient::addColorPoint(float red, float green, float blue, float value)
    {
        for (unsigned int i = 0; i < color.size(); i++)
        {
            if (value < color[i].val)
            {
                color.insert(color.begin() + i, ColorPoint(red, green, blue, value));
                return;
            }
        }
        color.push_back(ColorPoint(red, green, blue, value));
    }

    //-- Inserts a new color point into its correct position:
    void ColorGradient::clearGradient() { color.clear(); }

    //-- Places a 5 color heapmap gradient into the "color" vector:
    void ColorGradient::createDefaultHeatMapGradient()
    {
        color.clear();
        color.push_back(ColorPoint(0, 0, 1, 0.0f));  // Blue.
        color.push_back(ColorPoint(0, 1, 1, 0.25f)); // Cyan.
        color.push_back(ColorPoint(0, 1, 0, 0.5f));  // Green.
        color.push_back(ColorPoint(1, 1, 0, 0.75f)); // Yellow.
        color.push_back(ColorPoint(1, 0, 0, 1.0f));  // Red.
    }

    //-- Inputs a (value) between 0 and 1 and outputs the (red), (green) and (blue)
    //-- values representing that position in the gradient.
    void ColorGradient::getColorAtValue(const float value, float &red, float &green, float &blue)
    {
        if (color.size() == 0)
            return;

        for (unsigned int i = 0; i < color.size(); i++)
        {
            ColorPoint &currC = color[i];
            if (value < currC.val)
            {
                ColorPoint &prevC = color[max(0, (int)i - 1)];
                float valueDiff = (prevC.val - currC.val);
                float fractBetween = (valueDiff == 0) ? 0 : (value - currC.val) / valueDiff;
                red = (prevC.r - currC.r) * fractBetween + currC.r;
                green = (prevC.g - currC.g) * fractBetween + currC.g;
                blue = (prevC.b - currC.b) * fractBetween + currC.b;
                return;
            }
        }
        red = color.back().r;
        green = color.back().g;
        blue = color.back().b;
        return;
    }

} // namespace