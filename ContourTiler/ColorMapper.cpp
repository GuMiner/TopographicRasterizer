#include <cmath>
#include "ColorMapper.h"

void ColorMapper::MapColor(double percent, unsigned char* red, unsigned char* green, unsigned char* blue)
{
    // Do a HSV -> RGB conversion.
    double quadrant = percent * 6.0;
    double chroma = 1.0; // Saturation and value == 1
    double intermediate = chroma * (1 - (std::fmod(quadrant, 2) - 1));
    int scale = 255;
    if (quadrant < 1)
    {
        *red = (int)(scale * chroma);
        *green = (int)(scale * intermediate);
        *blue = 0;
    }
    else if (quadrant < 2)
    {
        *red = (int)(scale * intermediate);
        *green = (int)(scale * chroma);
        *blue = 0;
    }
    else if (quadrant < 3)
    {
        *red = 0;
        *green = (int)(scale * chroma);
        *blue = (int)(scale * intermediate);
    }
    else if (quadrant < 4)
    {
        *red = 0;
        *green = (int)(scale * intermediate);
        *blue = (int)(scale * chroma);
    }
    else if (quadrant < 5)
    {
        *red = (int)(scale * intermediate);
        *green = 0;
        *blue = (int)(scale * chroma);
    }
    else
    {
        *red = (int)(scale * chroma);
        *green = 0;
        *blue = (int)(scale * intermediate);
    }
}