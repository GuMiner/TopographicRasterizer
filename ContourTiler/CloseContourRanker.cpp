#include <cmath>
#include "CloseContourRanker.h"

CloseContourRanker::CloseContourRanker()
    : closestLine(CloseContourLine()), secondClosestLine(CloseContourLine()), thirdClosestLine(CloseContourLine())
{
}

bool CloseContourRanker::ResortIfIdentical(CloseContourLine contourLine)
{
    if (std::abs(closestLine.elevation - contourLine.elevation) < 0.001f)
    {
        // Identical, resorting done.
        if (contourLine.distanceSqd < closestLine.distanceSqd)
        {
            closestLine.distanceSqd = contourLine.distanceSqd;
        }

        return true;
    }
    else if (std::abs(secondClosestLine.elevation - contourLine.elevation) < 0.001f)
    {
        if (contourLine.distanceSqd < secondClosestLine.distanceSqd)
        {
            secondClosestLine.distanceSqd = contourLine.distanceSqd;
            if (secondClosestLine.distanceSqd < closestLine.distanceSqd)
            {
                // Swap
                CloseContourLine other;
                other.CopyFrom(closestLine);
                closestLine.CopyFrom(secondClosestLine);
                secondClosestLine.CopyFrom(other);
            }
        }

        return true;
    }
    else if (std::abs(thirdClosestLine.elevation - contourLine.elevation) < 0.001f)
    {
        if (contourLine.distanceSqd < thirdClosestLine.distanceSqd)
        {
            thirdClosestLine.distanceSqd = contourLine.distanceSqd;
            if (thirdClosestLine.distanceSqd < closestLine.distanceSqd)
            {
                // Swap and move down #2
                CloseContourLine other;
                other.CopyFrom(closestLine);
                closestLine.CopyFrom(thirdClosestLine);
                thirdClosestLine.CopyFrom(secondClosestLine);
                secondClosestLine.CopyFrom(other);
            }
            else if (thirdClosestLine.distanceSqd < secondClosestLine.distanceSqd)
            {
                CloseContourLine other;
                other.CopyFrom(secondClosestLine);
                secondClosestLine.CopyFrom(thirdClosestLine);
                thirdClosestLine.CopyFrom(other);
            }
        }

        return true;
    }

    
    return false;
}

void CloseContourRanker::AddElevationToRank(const CloseContourLine& contourLine)
{
    if (ResortIfIdentical(contourLine))
    {
        return;
    }

    // Not identical, figure out if it we need to insert this contour line anywhere.

    // Handle each contour sequentially, filling it in automatically if empty.
    if (!closestLine.populated)
    {
        closestLine.CopyFrom(contourLine);
        return;
    }
    else if (contourLine.distanceSqd < closestLine.distanceSqd)
    {
        // Move all down.
        thirdClosestLine.CopyFrom(secondClosestLine);
        secondClosestLine.CopyFrom(closestLine);
        closestLine.CopyFrom(contourLine);
        return;
    }

    if (!secondClosestLine.populated)
    {
        secondClosestLine.CopyFrom(contourLine);
        return;
    }
    else if (contourLine.distanceSqd < secondClosestLine.distanceSqd)
    {
        // Move second and third down.
        thirdClosestLine.CopyFrom(secondClosestLine);
        secondClosestLine.CopyFrom(contourLine);
        return;
    }

    if (!thirdClosestLine.populated || contourLine.distanceSqd < thirdClosestLine.distanceSqd)
    {
        thirdClosestLine.CopyFrom(contourLine);
        return;
    }
}

bool CloseContourRanker::FilledSufficientLines() const
{
    return closestLine.populated && secondClosestLine.populated;
}

double CloseContourRanker::GetWeightedElevation() const
{
    // We're guaranteed to have something in the closest line, but nothing in the other two.
    double elevation = 0;
    double inverseWeights = 0;

    // Double the sqrt for a less drastic flow.
    double distCL = closestLine.distanceSqd;
    elevation += closestLine.elevation / distCL;
    inverseWeights += 1.0 / distCL;

    if (secondClosestLine.populated)
    {
        distCL = secondClosestLine.distanceSqd;
        elevation += secondClosestLine.elevation / distCL;
        inverseWeights += 1.0 / distCL;
    }

    if (thirdClosestLine.populated)
    {
        distCL = thirdClosestLine.distanceSqd;
        elevation += thirdClosestLine.elevation / distCL;
        inverseWeights += 1.0 / distCL;
    }

    return elevation / inverseWeights;
}