#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include "Definitions.h"
#include "LineStripLoader.h"
#include "QuadExclusions.h"
#include "Quadtree.h"

class Rasterizer
{
    LineStripLoader* lineStrips;
    QuadExclusions* quadExclusions;
    Quadtree quadtree;

    // The number of quadtree xy grid spaces.
    int size;

    sf::Vector2i GetQuadtreeSquare(Point givenPoint);

    // Gets the closest distance from a point to a line ensuring we account for endpoints.
    decimal GetLineDistanceSqd(Index idx, Point point);

    // Adds an area if it is valid.
    void AddIfValid(int xP, int yP, std::vector<sf::Vector2i>& searchQuads);

    // Adds areas to search given the current point and distance away from it.
    void AddAreasToSearch(int distance, sf::Vector2i startQuad, std::vector<sf::Vector2i>& searchQuads);

    // Returns the height of the closest point to the specified coordinates.
    decimal FindClosestPoint(Point point);

    // Rasterizes a range of columns to improve perf.
    void RasterizeColumnRange(decimal leftOffset, decimal topOffset, decimal effectiveSize, int startColumn, int columnCount, double** rasterStore, double* minElevation, double* maxElevation);

    // Rasterizes a range of lines to improve perf.
    void RasterizeLineColumnRange(decimal leftOffset, decimal topOffset, decimal effectiveSize, int startColumn, int columnCount, double** rasterStore);

public:
    Rasterizer(LineStripLoader* lineStripLoader, QuadExclusions* quadExclusions, int size);

    // Setup to be done before rasterization can be performed.
    void Setup();

    // Rasterizes the area, filling in the raster store.
    void Rasterize(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore, double& minElevation, double& maxElevation);

    // Rasterizes in lines with full whiteness.
    void LineRaster(decimal leftOffset, decimal topOffset, decimal effectiveSize, double** rasterStore);

    virtual ~Rasterizer();
};

