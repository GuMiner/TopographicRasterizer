#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include "ElevationComputer.h"

ElevationComputer::ElevationComputer(Point point)
    : point(point)
{
    for (int i = 0; i < MaxRegions; i++)
    {
        lineRegions[i].IsPopulated = false;
    }
}

void ElevationComputer::GetClosestPointOnLine(Point point, Point start, Point end, Point* closestPoint)
{
    Point startToEnd(end.x - start.x, end.y - start.y);
    double startEndLengthSqd = pow(startToEnd.x, 2) + pow(startToEnd.y, 2);

    // Taking the dot product of the start-to-point vector with the (normalized) start-to-end vector.
    Point startToPoint(point.x - start.x, point.y - start.y);
    double projectionFraction = (startToPoint.x * startToEnd.x + startToPoint.y * startToEnd.y) / startEndLengthSqd;

    if (projectionFraction > 0 && projectionFraction < 1)
    {
        closestPoint->x = start.x + startToEnd.x * projectionFraction;
        closestPoint->y = start.y + startToEnd.y * projectionFraction;
    }
    else if (projectionFraction < 0)
    {
        closestPoint->x = start.x;
        closestPoint->y = start.y;
    }
    else
    {
        closestPoint->x = end.x;
        closestPoint->y = end.y;
    }
}

double ElevationComputer::ComputeAngle(Point point, Point otherPoint)
{
    double angle = std::atan2(otherPoint.y - point.y, otherPoint.x - point.x);
    if (angle < 0)
    {
        angle += 2 * M_PI;
    }

    return angle;
}

void ElevationComputer::ProcessLine(Point start, Point end, double elevation)
{
    // Lines cannot cross each other, so just find the minimum distance.
    // This distorts the geometry in an allowable manner.
    Point closestPoint = Point();
    ElevationComputer::GetClosestPointOnLine(this->point, start, end, &closestPoint);
    double angle = ElevationComputer::ComputeAngle(point, closestPoint);
    int quadrant = (int)(((double)MaxRegions * angle) / (2.0 * M_PI));
    if (quadrant >= MaxRegions)
    {
        quadrant = MaxRegions - 1;
    }

    double distanceSqd = pow(closestPoint.x - point.x, 2) + pow(closestPoint.y - point.y, 2);
    if (!lineRegions[quadrant].IsPopulated)
    {
        lineRegions[quadrant].IsPopulated = true;
        lineRegions[quadrant].DistanceSqd = distanceSqd;
        lineRegions[quadrant].Elevation = elevation;
    }
    else if (lineRegions[quadrant].DistanceSqd > distanceSqd)
    {
        lineRegions[quadrant].DistanceSqd = distanceSqd;
        lineRegions[quadrant].Elevation = elevation;
    }
    else
    {
        // The quadrant is already populated and closer than the given line, so continue.
    }
}

bool ElevationComputer::HasSufficientData() const
{
    // If two opposing quadrants are covered, we're done.
    // For example, for 0,1,2,3, 4,5,6,7,8,9, 0/5, 1/6, 2/7, 3/8, 4/9 are the opposing pairs.
    const int halfMaxRegions = MaxRegions / 2;
    for (int i = 0; i < MaxRegions; i++)
    {
        if (!lineRegions[i].IsPopulated)
        {
            return false;
        }
    }

    return true;
}

double ElevationComputer::GetWeightedElevation() const
{
    // Weight each elevation by its distance squared.
    double elevation = 0;
    double inverseWeights = 0;

    for (int i = 0; i < MaxRegions; i++)
    {
        if (lineRegions[i].IsPopulated)
        {
            elevation += lineRegions[i].Elevation / lineRegions[i].DistanceSqd;
            inverseWeights += 1.0 / lineRegions[i].DistanceSqd;
        }
    }

    return elevation / inverseWeights;
}