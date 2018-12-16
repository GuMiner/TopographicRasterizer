#pragma once
#include "Definitions.h"

struct CloseContourLine
{
    decimal distanceSqd;
    int elevationId;
    decimal elevation;

    CloseContourLine()
        : elevationId(-1)
    {
    }

    CloseContourLine(decimal distanceSqd, int elevationId, decimal elevation)
        : distanceSqd(distanceSqd), elevationId(elevationId), elevation(elevation)
    {
    }

    void CopyFrom(CloseContourLine other)
    {
        elevation = other.elevation;
        distanceSqd = other.distanceSqd;
        elevationId = other.elevationId;
    }
};

class CloseContourRanker
{
    CloseContourLine closestLine;
    CloseContourLine secondClosestLine;
    CloseContourLine thirdClosestLine;

    bool ResortIfIdentical(CloseContourLine contourLine);

public:
    CloseContourRanker();
    void AddElevationToRank(const CloseContourLine& contourLine);
    bool FilledSufficientLines() const;
    decimal GetWeightedElevation() const;
    
};

