#pragma once
class ColorMapper
{
public:
    // Maps a percent from 0-1 to a nice color.
    static void MapColor(double percent, unsigned char* red, unsigned char* green, unsigned char* blue);
};

