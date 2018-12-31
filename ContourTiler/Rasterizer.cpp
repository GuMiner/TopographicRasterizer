#include <cmath>
#include <thread>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <immintrin.h>
#include <xmmintrin.h>
#include "CloseContourRanker.h"
#include "Rasterizer.h"

std::mutex logMutex;

Rasterizer::Rasterizer(LineStripLoader* lineStripLoader)
    : lineStrips(lineStripLoader), quadtree()
{
}

// Map the given (normalized) points to 0-(size - 1( (defaults to 0-9) for block-based lookup.
sf::Vector2i Rasterizer::GetQuadtreeSquare(Point givenPoint)
{
    return sf::Vector2i(
        std::min((int)(givenPoint.x * (double)size), size - 1),
        std::min((int)(givenPoint.y * (double)size), size - 1));
}

sf::Vector2i Rasterizer::GetQuadtreeSquare(LowResPoint givenPoint)
{
    return sf::Vector2i(
        std::min((int)(givenPoint.x * (float)size), size - 1),
        std::min((int)(givenPoint.y * (float)size), size - 1));
}

void Rasterizer::Setup(Settings* settings)
{
    this->settings = settings;
    this->size = this->settings->RegionSize;

    std::cout << "Initializing point lookup quadtree..." << std::endl;
    quadtree.InitializeQuadtree(this->size);

    // Now fill in all the quadtree files with the indexes of all the lines within the area.
    // TODO: This *has* to include lines that pass through, but don't end in regions.
    for (int i = 0; i < lineStrips->lineStrips.size(); i++)
    {
        if (this->settings->IsHighResolution)
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
            }
        }
        else
        {
            // TODO: Deduplicate
            std::vector<LowResPoint>& points = lineStrips->lineStrips[i].lowResPoints;
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
            }
        }

        if (i % (lineStrips->lineStrips.size() / 10) == 0)
        {
            std::cout << "  Processed line strip " << i << " of " << lineStrips->lineStrips.size() << std::endl;
        }
    }

    std::cout << "Quadtree initialized!" << std::endl;
}

// Same as the above but treats the index as a line.
double Rasterizer::GetLineDistanceSqd(Index idx, Point point)
{
    if (this->settings->IsHighResolution)
    {
        Point& start = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx];
        Point& end = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1];

        Point startToEnd(end.x - start.x, end.y - start.y);
        double startEndLengthSqd = pow(startToEnd.x, 2) + pow(startToEnd.y, 2);

        // Taking the dot product of the start-to-point vector with the (normalized) start-to-end vector.
        Point startToPoint(point.x - start.x, point.y - start.y);
        double projectionFraction = (startToPoint.x * startToEnd.x + startToPoint.y * startToEnd.y) / startEndLengthSqd;

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
    }
    else
    {
        LowResPoint& start = lineStrips->lineStrips[idx.stripIdx].lowResPoints[idx.pointIdx];
        LowResPoint& end = lineStrips->lineStrips[idx.stripIdx].lowResPoints[idx.pointIdx + 1];

        LowResPoint startToEnd(end.x - start.x, end.y - start.y);
        double startEndLengthSqd = pow(startToEnd.x, 2) + pow(startToEnd.y, 2);

        // Taking the dot product of the start-to-point vector with the (normalized) start-to-end vector.
        LowResPoint startToPoint((float)(point.x - start.x), (float)(point.y - start.y));
        double projectionFraction = (startToPoint.x * startToEnd.x + startToPoint.y * startToEnd.y) / startEndLengthSqd;

        if (projectionFraction > 0 && projectionFraction < 1)
        {
            LowResPoint closestPoint((float)(start.x + startToEnd.x * projectionFraction), (float)(start.y + startToEnd.y * projectionFraction));
            return pow(closestPoint.x - point.x, 2) + pow(closestPoint.y - point.y, 2);
        }
        else if (projectionFraction < 0)
        {
            return pow(startToPoint.x, 2) + pow(startToPoint.y, 2);
        }

        return pow(end.x - point.x, 2) + pow(end.y - point.y, 2);
    }
}

void Rasterizer::AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads)
{
    sf::Vector2i pt(xP, yP);
    if (xP >= 0 && yP >= 0 && xP < size && yP < size && quadtree.ElementsInQuad(pt) != 0)
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

double Rasterizer::FindClosestPoint(Point point)
{
    sf::Vector2i quadSquare = GetQuadtreeSquare(point);

    // Loop forever as we are guaranteed to eventually find a point.
    int gridDistance = 1;
    std::vector<sf::Vector2i> searchQuads;

    int maxIterations = 90; // Also empirical, works ok.
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
            int indexCount = (int)quadtree.ElementsInQuad(searchQuads[k]);
            for (size_t i = 0; i < indexCount; i++)
            {
                Index index = quadtree.GetIndexFromQuad(searchQuads[k], (int)i);
                
                nextLine.distanceSqd = GetLineDistanceSqd(index, point);
                nextLine.elevation = (double)lineStrips->lineStrips[index.stripIdx].elevation;
                if (nextLine.distanceSqd < 1e-12)
                {
                    // Exit early if we're effectively right on the line.
                    return nextLine.elevation;
                }

                // Add to the ranker, which manages priority of lines.
                nextLine.populated = true;
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

    // Return something invalid if we never found a line.
    return 2e8;
}

// Rasterizes a range of columns to improve perf.
void Rasterizer::RasterizeColumnRange(double leftOffset, double topOffset, double effectiveSize, int startColumn, int columnCount, double** rasterStore)
{
    for (int i = startColumn; i < startColumn + columnCount; i++)
    {
        for (int j = 0; j < size; j++)
        {
            double x = leftOffset + ((double)i / (double)size) * effectiveSize;
            double y = topOffset + ((double)j / (double)size) * effectiveSize;

            Point point(x, y);
            double elevation = FindClosestPoint(point);
            (*rasterStore)[i + j * size] = elevation;
        }
    }

    // Ensure log messages are rendered neatly.
    logMutex.lock();
    std::cout << "  Rasterization from " << startColumn << " to " << (startColumn + columnCount) << " complete." << std::endl;
    logMutex.unlock();
}

void Rasterizer::Rasterize(double leftOffset, double topOffset, double effectiveSize, double** rasterStore)
{
    std::cout << "Region Rasterizing..." << std::endl;

    // Split apart rasterization across all cores - 1, or 7 if we can't find hardware cores.
    unsigned int hardwareThreads = std::thread::hardware_concurrency();
    int splitFactor = hardwareThreads < 3 ? 7 : hardwareThreads - 1;

    std::thread** threads = new std::thread*[splitFactor];

    int range = size / splitFactor;
    for (int i = 0; i < splitFactor; i++)
    {
        int actualRange = (i == splitFactor - 1) ? (size - range * splitFactor) + range : range;
        threads[i] = new std::thread(&Rasterizer::RasterizeColumnRange, this, leftOffset, topOffset, effectiveSize, i * range, actualRange, rasterStore);
    }

    for (int i = 0; i < splitFactor; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    delete[] threads;
    std::cout << "Region Rasterization complete." << std::endl;
}

// Rasterizes a range of lines to improve perf.
void Rasterizer::RasterizeLineColumnRange(double leftOffset, double topOffset, double effectiveSize, int startColumn, int columnCount, double** rasterStore)
{
    for (int i = startColumn; i < startColumn + columnCount; i++)
    {
        for (int j = 0; j < size; j++) // Column from top to bottom.
        {
            double x = leftOffset + ((double)i / (double)size) * effectiveSize;
            double y = topOffset + ((double)j / (double)size) * effectiveSize;
            double wiggleDistSqd = pow(effectiveSize / (double)size, 2)*2;

            Point point(x, y);
            sf::Vector2i quadSquare = GetQuadtreeSquare(point);

            bool filled = false;
            for (size_t k = 0; k < quadtree.ElementsInQuad(quadSquare); k++)
            {
                Index index = quadtree.GetIndexFromQuad(quadSquare, (int)k);

                double lineDistSqd = GetLineDistanceSqd(index, point);
                if (lineDistSqd < wiggleDistSqd)
                {
                    filled = true;
                    break;
                }
            }

            (*rasterStore)[i + j * size] = filled ? 1 : 0;
        }
    }

    logMutex.lock();
    std::cout << "  Rasterization from " << startColumn << " to " << (startColumn + columnCount) << " complete." << std::endl;
    logMutex.unlock();
}

void Rasterizer::LineRaster(double leftOffset, double topOffset, double effectiveSize, double** rasterStore)
{
    std::cout << "Line Rasterizing..." << std::endl;
 
    // Split apart rasterization across all cores - 1, or 7 if we can't find hardware cores.
    unsigned int hardwareThreads = std::thread::hardware_concurrency();
    int splitFactor = hardwareThreads < 3 ? 7 : hardwareThreads - 1;

    std::thread** threads = new std::thread*[splitFactor];

    int range = size / splitFactor;
    for (int i = 0; i < splitFactor; i++)
    {
        int actualRange = (i == splitFactor - 1) ? (size - range * splitFactor) + range : range;
        threads[i] = new std::thread(&Rasterizer::RasterizeLineColumnRange, this, leftOffset, topOffset, effectiveSize, i * range, actualRange, rasterStore);
    }

    for (int i = 0; i < splitFactor; i++)
    {
        threads[i]->join();
        delete threads[i];
    }

    delete[] threads;
    std::cout << "Line rasterization complete." << std::endl;
}