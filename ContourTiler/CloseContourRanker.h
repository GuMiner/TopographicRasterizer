#pragma once

struct CloseContourLine
{
    bool populated;
    double distanceSqd;
    double elevation;

    CloseContourLine()
        : populated(false)
    {
    }

    CloseContourLine(double distanceSqd, double elevation)
        : distanceSqd(distanceSqd), elevation(elevation), populated(false)
    {
    }

    void CopyFrom(CloseContourLine other)
    {
        elevation = other.elevation;
        distanceSqd = other.distanceSqd;
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
    double GetWeightedElevation() const;
    
};

