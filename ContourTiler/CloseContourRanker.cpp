#include "CloseContourRanker.h"



CloseContourRanker::CloseContourRanker()
    : closestLine(CloseContourLine()), secondClosestLine(CloseContourLine()), thirdClosestLine(CloseContourLine())
{
}

bool CloseContourRanker::ResortIfIdentical(CloseContourLine contourLine)
{
    if (closestLine.elevationId == contourLine.elevationId)
    {
        // Identical, resorting done.
        if (contourLine.distanceSqd < closestLine.distanceSqd)
        {
            closestLine.distanceSqd = contourLine.distanceSqd;
        }

        return true;
    }
    else if (secondClosestLine.elevationId == contourLine.elevationId)
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
    else if (thirdClosestLine.elevationId == contourLine.elevationId)
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
    if (closestLine.elevationId == -1)
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

    if (secondClosestLine.elevationId == -1)
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

    if (thirdClosestLine.elevationId == -1 || contourLine.distanceSqd < thirdClosestLine.distanceSqd)
    {
        thirdClosestLine.CopyFrom(contourLine);
        return;
    }
}

bool CloseContourRanker::FilledSufficientLines() const
{
    return closestLine.elevationId != -1 && secondClosestLine.elevationId != -1; // && thirdClosestLine.elevationId != -1;
}

decimal CloseContourRanker::GetWeightedElevation() const
{
    // We're guaranteed to have something in the closest line, but nothing in the other two.
    decimal elevation = 0;
    decimal inverseWeights = 0;

    // Double the sqrt for a less drastic flow.
    decimal distCL = closestLine.distanceSqd;
    elevation += closestLine.elevation / distCL;
    inverseWeights += (decimal)1.0 / distCL;

    if (secondClosestLine.elevationId != -1)
    {
        distCL = secondClosestLine.distanceSqd;
        elevation += secondClosestLine.elevation / distCL;
        inverseWeights += (decimal)1.0 / distCL;
    }

    if (thirdClosestLine.elevationId != -1)
    {
        distCL = thirdClosestLine.distanceSqd;
        elevation += thirdClosestLine.elevation / distCL;
        inverseWeights += (decimal)1.0 / distCL;
    }

    return elevation / inverseWeights;
}