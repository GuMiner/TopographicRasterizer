#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include "LineStripLoader.h"
#include "Quadtree.h"

class Rasterizer
{
    Settings* settings;
    LineStripLoader* lineStrips;
    Quadtree quadtree;

    // The number of quadtree xy grid spaces.
    int size;

    // Map the given (normalized) points to 0-(size - 1) (defaults to 0-9) for block-based lookup.
    template <typename T>
    sf::Vector2i GetQuadtreeSquare(T givenPoint)
    {
        return sf::Vector2i(
            std::min((int)(givenPoint.x * (double)size), size - 1),
            std::min((int)(givenPoint.y * (double)size), size - 1));
    }

    template <typename T>
    void AddPointsToQuadtree(int lineStripIndex, std::vector<T> points)
    {
        for (unsigned int j = 0; j < points.size() - 1; j++)
        {
            sf::Vector2i quadStart = GetQuadtreeSquare(points[j]);
            sf::Vector2i quadEnd = GetQuadtreeSquare(points[j + 1]);
            Index index(lineStripIndex, j);

            // Add the start
            quadtree.AddToIndex(quadStart, index);

            // Add where the line intersects to the quadtree, iterating in the length where our V1 algorithm works.
            sf::Vector2i distance = quadEnd - quadStart;


            if (quadEnd.x == quadStart.x && quadEnd.y == quadStart.y)
            {
                continue;
            }
            else if (std::abs(distance.x) >= std::abs(distance.y))
            {
                // Iterate X, going in the negative or positive direction appropriately.
                int x = quadStart.x;
                const int increment = quadStart.x < quadEnd.x ? 1 : -1;
                const float yDelta = (float)distance.y / (float)std::abs(distance.x);

                int currentX = 0;
                while (x != quadEnd.x)
                {
                    x += increment;
                    ++currentX;

                    int y = (int)(yDelta * currentX + quadStart.y);

                    // V1: Add to both Y plus and minus 1 to be pessimistic
                    quadtree.AddToIndex(sf::Vector2i(x, y), index);
                    quadtree.AddToIndex(sf::Vector2i(x, y + 1), index);
                    quadtree.AddToIndex(sf::Vector2i(x, y - 1), index);
                }
            }
            else
            {
                // Iterate Y, going in the negative or positive direction appropriately.
                int y = quadStart.y;
                const int increment = quadStart.y < quadEnd.y ? 1 : -1;
                const float xDelta = (float)distance.x / (float)std::abs(distance.y);

                int currentY = 0;
                while (y != quadEnd.y)
                {
                    y += increment;
                    ++currentY;

                    int x = (int)(xDelta * currentY + quadStart.y);

                    // V1: Add to both X plus and minus 1 to be pessimistic
                    quadtree.AddToIndex(sf::Vector2i(x, y), index);
                    quadtree.AddToIndex(sf::Vector2i(x + 1, y), index);
                    quadtree.AddToIndex(sf::Vector2i(x - 1, y), index);
                }
            }
        }
    }

    // Gets the closest distance from a point to a line ensuring we account for endpoints.
    double GetLineDistanceSqd(Index idx, Point point);

    // Adds an area if it is valid.
    void AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads);

    // Adds areas to search given the current point and distance away from it.
    void AddAreasToSearch(int distance, sf::Vector2i startQuad, std::vector<sf::Vector2i>& searchQuads);

    // Returns the height of the closest point to the specified coordinates.
    double FindClosestPoint(Point point);

    // Rasterizes a range of columns to improve perf.
    void RasterizeColumnRange(double leftOffset, double topOffset, double effectiveSize, int startColumn, int columnCount, double** rasterStore);

    // Rasterizes a range of lines to improve perf.
    void RasterizeLineColumnRange(double leftOffset, double topOffset, double effectiveSize, int startColumn, int columnCount, double** rasterStore);

public:
    Rasterizer(LineStripLoader* lineStripLoader);

    // Setup to be done before rasterization can be performed.
    void Setup(Settings* settings);

    // Rasterizes the area, filling in the raster store.
    void Rasterize(double leftOffset, double topOffset, double effectiveSize, double** rasterStore);

    // Rasterizes in lines with full whiteness.
    void LineRaster(double leftOffset, double topOffset, double effectiveSize, double** rasterStore);
};

