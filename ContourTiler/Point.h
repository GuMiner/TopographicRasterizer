#pragma once

struct Point
{
    double x, y;

    Point()
    { }

    Point(double xx, double yy) : x(xx), y(yy)
    { }
};

struct LowResPoint
{
    float x, y;

    LowResPoint()
    { }

    LowResPoint(float xx, float yy) : x(xx), y(yy)
    { }
};