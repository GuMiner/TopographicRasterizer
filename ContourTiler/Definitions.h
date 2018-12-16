#pragma once
#include <vector>

namespace Constant
{
    // RasterFolder defined elsewhere!
    const std::string ContourFile = "hardcodedlist";
    
    // const double XMin = // 326851.6216;
    // const double XMax = // 985010.1055;  // 658158
    //                     // 
    // const double YMin = // 2086366.2587; // 381263
    // const double YMax = // 2744524.2587;
    const double ZMin = 0;
    const double ZMax = 13900;

    // const char* RasterFolder = "rasters";
    // const std::string ContourFile = "data.bin";
    // 
    // const double XMin = 1214976.7500000000;
    // const double XMax = 1583489.0000000000;
    // 
    // const double YMin = 22489.217250004411;
    // const double YMax = 360015.64550000429;
    // const double ZMin = -900.00000000000000;
    // const double ZMax = 7960.0000000000000;
}

// Lines are construed to be from the current point to the next point.
struct Index
{
    int stripIdx;
    int pointIdx;

    Index()
    { }

    Index(int si, int pi) : stripIdx(si), pointIdx(pi)
    { }
};

// If you have a machine with a lot of memory > 32 Gb, toggle this to double for accurate data precision.
#define decimal double 

// Point data format.
struct Point
{
    decimal x, y;

    Point()
    { }

    Point(decimal xx, decimal yy) : x(xx), y(yy)
    { }
};

// Line data format.
struct LineStrip
{
    double elevation;
    int elevationId;
    std::vector<Point> points;
};