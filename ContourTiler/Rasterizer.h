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

    sf::Vector2i GetQuadtreeSquare(Point givenPoint);
    sf::Vector2i GetQuadtreeSquare(LowResPoint givenPoint);

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

