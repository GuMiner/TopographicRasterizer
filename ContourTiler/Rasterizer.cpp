#include <cmath>
#include <thread>
#include <iostream>
#include <limits>
#include <map>
#include <immintrin.h>
#include <xmmintrin.h>
#include "CloseContourRanker.h"
#include "Rasterizer.h"

double toggle;

Rasterizer::Rasterizer(LineStripLoader* lineStripLoader, QuadExclusions* quadExclusions, int size)
    : lineStrips(lineStripLoader), quadExclusions(quadExclusions), quadtree(size), size(size)
{
    toggle = 0;
}


sf::Vector2i Rasterizer::GetQuadtreeSquare(Point givenPoint)
{
    return sf::Vector2i(
        std::min((int)(givenPoint.x * (decimal)size + (decimal)toggle), size - 1),
        std::min((int)(givenPoint.y * (decimal)size + (decimal)toggle), size - 1));
}

void Rasterizer::Setup()
{
    quadtree.InitializeQuadtree();

    // Now fill in all the quadtree files with the indexes of all the lines within the area.
    long pointCount = 0;
    for (int i = 0; i < lineStrips->lineStrips.size(); i++)
    {
        std::vector<Point>& points = lineStrips->lineStrips[i].points;

        for (unsigned int j = 0; j < points.size() - 1; j++)
        {
            sf::Vector2i quadStart = GetQuadtreeSquare(points[j]);
            sf::Vector2i quadEnd = GetQuadtreeSquare(points[j + 1]);
            Index index(i, j);

            quadtree.AddToIndex(quadStart, index);

            if (quadEnd.x != quadStart.x || quadEnd.y != quadStart.y)
            {
                quadtree.AddToIndex(quadEnd, index);
            }

            ++pointCount;
            if (pointCount % 1000000 == 0)
            {
                std::cout << "Quadtree processed point " << pointCount << ". (" << (float)pointCount / 1.5e7f << "%)" << std::endl;
            }
        }
    }

    toggle = 0;

    std::cout << "Quadtree initialized!" << std::endl;
}

// Same as the above but treats the index as a line.
decimal Rasterizer::GetLineDistanceSqd(Index idx, Point point)
{
    Point& start = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx];
    Point& end = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1];

    Point startToEnd(end.x - start.x, end.y - start.y);
    decimal startEndLengthSqd = pow(startToEnd.x, 2) + pow(startToEnd.y, 2);

    // Taking the dot product of the start-to-point vector with the (normalized) start-to-end vector.
    Point startToPoint(point.x - start.x, point.y - start.y);
    decimal projectionFraction = (startToPoint.x * startToEnd.x + startToPoint.y * startToEnd.y) / startEndLengthSqd;

    if (projectionFraction > 0 && projectionFraction < 1)
    {
        Point closestPoint(start.x + startToEnd.x * projectionFraction, start.y + startToEnd.y * projectionFraction);
        return pow(closestPoint.x - point.x, 2) + pow(closestPoint.y - point.y, 2);
    }
    else if (projectionFraction < 0)
    {
        return pow(startToPoint.x, 2) + pow(startToPoint.y, 2);
    }

    return pow(end.x - point.x, 2) + pow(end.y - point.y, 2);

    /*__m128 start;
    start.m128_f32[0] = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx].x;
    start.m128_f32[1] = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx].y;

    __m128 end;
    end.m128_f32[0] = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1].x;
    end.m128_f32[1] = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1].y;

    __m128 pt;
    pt.m128_f32[0] = point.x;
    pt.m128_f32[1] = point.y;

    __m128 startToEnd = _mm_sub_ps(start, end);
    __m128 startEndLengthSqd = _mm_mul_ps(startToEnd, startToEnd);
    startEndLengthSqd.m128_f32[0] += startEndLengthSqd.m128_f32[1];
    startEndLengthSqd.m128_f32[1] = startEndLengthSqd.m128_f32[0];

    // Taking the dot product of the start-to-point vector with the (normalized) start-to-end vector.
    __m128 startToPoint = _mm_sub_ps(pt, start);
    __m128 projectionFraction = _mm_mul_ps(startToPoint, startToEnd);
    projectionFraction.m128_f32[0] += projectionFraction.m128_f32[1];
    projectionFraction.m128_f32[1] = projectionFraction.m128_f32[0];

    projectionFraction = _mm_div_ps(projectionFraction, startEndLengthSqd);

    if (projectionFraction.m128_f32[0] > 0 && projectionFraction.m128_f32[0] < 1)
    {
        __m128 closestPoint = _mm_sub_ps(_mm_add_ps(start, _mm_mul_ps(startToEnd, projectionFraction)), pt);
        closestPoint = _mm_mul_ps(closestPoint, closestPoint);
        return closestPoint.m128_f32[0] + closestPoint.m128_f32[1];

    }
    else if (projectionFraction.m128_f32[0] < 0)
    {
        startToPoint = _mm_mul_ps(startToPoint, startToPoint);
        return start.m128_f32[0] + start.m128_f32[1];
    }

    end = _mm_sub_ps(end, pt);
    end = _mm_mul_ps(end, end);
    return end.m128_f32[0] + end.m128_f32[1];*/

    /*// Loads start into the first two bytes, end into the next two.
    __m128 start = _mm_loadu_ps(&(lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx].x));

    __m128 end;
    end.m128_f32[0] = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1].x;
    end.m128_f32[1] = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1].y;
    end.m128_f32[2] = 0;
    end.m128_f32[3] = 0;

    __m128 pt;
    pt.m128_f32[0] = point.x;
    pt.m128_f32[1] = point.y;
    pt.m128_f32[2] = point.x;
    pt.m128_f32[3] = point.y;

    __m128 startToEnd = _mm_sub_ps(start, end);
    __m128 startEndLengthSqd = _mm_mul_ps(startToEnd, startToEnd);
    startEndLengthSqd = _mm_set1_ps(startEndLengthSqd.m128_f32[0] + startEndLengthSqd.m128_f32[1]);

    // Taking the dot product of the start-to-point vector with the (normalized) start-to-end vector.
    __m128 startToPoint = _mm_sub_ps(pt, start);
    __m128 projectionFraction = _mm_mul_ps(startToPoint, startToEnd);
    projectionFraction = _mm_set1_ps(projectionFraction.m128_f32[0] + projectionFraction.m128_f32[1]);
    projectionFraction = _mm_div_ps(projectionFraction, startEndLengthSqd);

    if (projectionFraction.m128_f32[0] > 0 && projectionFraction.m128_f32[0] < 1)
    {
        __m128 closestPoint = _mm_sub_ps(_mm_add_ps(start, _mm_mul_ps(startToEnd, projectionFraction)), pt);
        closestPoint = _mm_mul_ps(closestPoint, closestPoint);
        return closestPoint.m128_f32[0] + closestPoint.m128_f32[1];
    }
    else if (projectionFraction.m128_f32[0] < 0)
    {
        startToPoint = _mm_mul_ps(startToPoint, startToPoint);
        return start.m128_f32[0] + start.m128_f32[1];
    }

    startToPoint = _mm_mul_ps(startToPoint, startToPoint);
    return end.m128_f32[2] + end.m128_f32[3];*/
}

void Rasterizer::AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads)
{
    sf::Vector2i pt(xP, yP);
    if (xP >= 0 && yP >= 0 && xP < size && yP < size && !quadExclusions->IsExcluded(pt) && quadtree.QuadSize(pt) != 0)
    {
        searchQuads.push_back(sf::Vector2i(xP, yP));
    }
}

void Rasterizer::AddAreasToSearch(int distance, sf::Vector2i startQuad, std::vector<sf::Vector2i>& searchQuads)
{
    if (distance == 1)
    {
        searchQuads.push_back(startQuad);
    }

    // Add the horizontal bars
    for (int i = startQuad.x - distance; i <= startQuad.x + distance; i++)
    {
        AddIfValid(i, startQuad.y + distance, searchQuads);
        AddIfValid(i, startQuad.y - distance, searchQuads);
    }

    // Add the vertical bars, skipping the corners that otherwise would be duplicated.
    for (int j = startQuad.y - (distance - 1); j <= startQuad.y + (distance - 1); j++)
    {
        AddIfValid(startQuad.x + distance, j, searchQuads);
        AddIfValid(startQuad.x - distance, j, searchQuads);
    }
}

decimal Rasterizer::FindClosestPoint(Point point)
{
    sf::Vector2i quadSquare = GetQuadtreeSquare(point);

    if (false) // quadExclusions->IsExcluded(quadSquare))
    {
        // The point the point is within is excluded. Return immediately.
        return 2e8;
    }

    // Loop forever as we are guaranteed to eventually find a point.
    int gridDistance = 1;
    std::vector<sf::Vector2i> searchQuads;

    int maxIterations = 90;
    bool foundAPoint = false;
    int overrun = 0;
    const int overrunLimit = 5; // Empirically determined to be 'ok'
    CloseContourRanker contourRanker = CloseContourRanker();
    CloseContourLine nextLine;
    while (maxIterations > 0)
    {
        --maxIterations;
        searchQuads.clear();
        AddAreasToSearch(gridDistance, quadSquare, searchQuads);

        for (int k = 0; k < searchQuads.size(); k++)
        {
            int indexCount = (int)quadtree.QuadSize(searchQuads[k]);
            for (size_t i = 0; i < indexCount; i++)
            {
                Index index = quadtree.GetIndexFromQuad(searchQuads[k], (int)i);
                
                nextLine.distanceSqd = GetLineDistanceSqd(index, point);
                nextLine.elevation = (decimal)lineStrips->lineStrips[index.stripIdx].elevation;
                if (nextLine.distanceSqd < 1e-12)
                {
                    return nextLine.elevation;
                }

                // Add to the ranker, which manages priority of lines.
                nextLine.elevationId = lineStrips->lineStrips[index.stripIdx].elevationId;
                contourRanker.AddElevationToRank(nextLine);
                foundAPoint = true;
            }
        }

        if (foundAPoint && (overrun >= overrunLimit || contourRanker.FilledSufficientLines()))
        {
            return contourRanker.GetWeightedElevation();
        }
        else if (foundAPoint)
        {
            // Overrun a few times to account for edge quadtree conditions. We overrun at least once if we're near a boundary zone.
            ++overrun;
        }

        // Increment the grids we search.
        ++gridDistance;
    }

    return 2e8;
}

// Rasterizes a range of columns to improve perf.
void Rasterizer::RasterizeColumnRange(decimal leftOffset, decimal topOffset, decimal effectiveSize, int startColumn, int columnCount, double** rasterStore, double* minElevation, double* maxElevation)
{
    for (int i = startColumn; i < startColumn + columnCount; i++)
    {
        for (int j = 0; j < size; j++)
        {
            decimal x = leftOffset + ((decimal)i / (decimal)size) * effectiveSize;
            decimal y = topOffset + ((decimal)j / (decimal)size) * effectiveSize;

            Point point(x, y);
            decimal elevation = FindClosestPoint(point);
            (*rasterStore)[i + j * size] = elevation;

            // Avoid scaling based on the exclusion lists.
            if (elevation < 1e8)
            {
                if (elevation < *minElevation)
                {
                    *minElevation = elevation;
                }

                if (elevation > *maxElevation)
                {
                    *maxElevation = elevation;
                }
            }
        }

        if (i % 50 == 0)
        {
            std::cout << "Rasterized " << i << " of " << size << std::endl;
        }
    }

    std::cout << "Thread from " << startColumn << " to " << (startColumn + columnCount) << " complete." << std::endl;
}

void Rasterizer::Rasterize(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore, double& minElevation, double& maxElevation)
{
    minElevation = std::numeric_limits<double>::max();
    maxElevation = std::numeric_limits<double>::min();

    const int splitFactor = 7;
    double minElevations[splitFactor];
    double maxElevations[splitFactor];
    std::thread* threads[splitFactor];

    int range = size / splitFactor;
    for (int i = 0; i < splitFactor; i++)
    {
        minElevations[i] = std::numeric_limits<double>::max();
        maxElevations[i] = std::numeric_limits<double>::min();

        int actualRange = (i == splitFactor - 1) ? (size - range * splitFactor) + range : range;
        threads[i] = new std::thread(&Rasterizer::RasterizeColumnRange, this, leftOffset, topOffset, effectiveSize, i * range, actualRange, rasterStore, &minElevations[i], &maxElevations[i]);
    }

    std::cout << "Rasterizing..." << std::endl;
    
    for (int i = 0; i < splitFactor; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    // Find the real min and max.
    for (int i = 0; i < splitFactor; i++)
    {
        if (minElevations[i] < minElevation)
        {
            minElevation = minElevations[i];
        }

        if (maxElevations[i] > maxElevation)
        {
            maxElevation = maxElevations[i];
        }
    }

    std::cout << "Rasterization complete." << std::endl;
}

// Rasterizes a range of lines to improve perf.
void Rasterizer::RasterizeLineColumnRange(decimal leftOffset, decimal topOffset, decimal effectiveSize, int startColumn, int columnCount, double** rasterStore)
{
    for (int i = startColumn; i < startColumn + columnCount; i++)
    {
        for (int j = 0; j < size; j++)
        {
            decimal x = leftOffset + ((decimal)i / (decimal)size) * effectiveSize;
            decimal y = topOffset + ((decimal)j / (decimal)size) * effectiveSize;
            decimal wiggleDistSqd = pow(effectiveSize / (decimal)size, 2);

            Point point(x, y);
            sf::Vector2i quadSquare = GetQuadtreeSquare(point);

            bool filled = false;
            for (size_t k = 0; k < quadtree.QuadSize(quadSquare); k++)
            {
                Index index = quadtree.GetIndexFromQuad(quadSquare, (int)k);

                decimal lineDistSqd = GetLineDistanceSqd(index, point);
                if (lineDistSqd < wiggleDistSqd)
                {
                    filled = true;
                    break;
                }
            }

            (*rasterStore)[i + j * size] = filled ? 1 : 0;
        }

        if (i % 20 == 0)
        {
            std::cout << "Line rendered " << i << " of " << size << std::endl;
        }
    }

    std::cout << "Thread from " << startColumn << " to " << (startColumn + columnCount) << " complete." << std::endl;
}

void Rasterizer::LineRaster(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore)
{
    // This could also be parallelized.
    std::cout << "Line Rasterizing..." << std::endl;
    
    const int splitFactor = 7;
    std::thread* threads[splitFactor];

    int range = size / splitFactor;
    for (int i = 0; i < splitFactor; i++)
    {
        int actualRange = (i == splitFactor - 1) ? (size - range * splitFactor) + range : range;
        threads[i] = new std::thread(&Rasterizer::RasterizeLineColumnRange, this, leftOffset, topOffset, effectiveSize, i * range, actualRange, rasterStore);
    }

    std::cout << "Rasterizing..." << std::endl;

    for (int i = 0; i < splitFactor; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    std::cout << "Rasterization complete." << std::endl;
}

Rasterizer::~Rasterizer()
{
}
