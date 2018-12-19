#pragma once
#include <vector>
#include "Point.h"

// Line data format.
struct LineStrip
{
    double elevation;
    std::vector<Point> points;
    std::vector<LowResPoint> lowResPoints;
};