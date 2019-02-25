#pragma once
#include "Point.h"

class DistanceElevation
{
public:
    double DistanceSqd;
    double Elevation;
    bool IsPopulated;
};

// Computes the elevation for the given point 
class ElevationComputer
{
    // Must be even!
    static const int MaxRegions = 10;

    Point point;
    
    DistanceElevation lineRegions[MaxRegions];
public:
    ElevationComputer(Point point);

    static void GetClosestPointOnLine(Point point, Point start, Point end, Point* closestPoint);
    static double ComputeAngle(Point point, Point otherPoint);

    void ProcessLine(Point start, Point end, double elevation);
    bool HasSufficientData() const;
    double GetWeightedElevation() const;
};