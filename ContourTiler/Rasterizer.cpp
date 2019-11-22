#include <cmath>
#include <thread>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <immintrin.h>
#include <xmmintrin.h>
#include "ElevationComputer.h"
#include "Rasterizer.h"

std::mutex logMutex;

Rasterizer::Rasterizer(LineStripLoader* lineStripLoader)
    : lineStrips(lineStripLoader), quadtree()
{
}

void Rasterizer::Setup(Settings* settings)
{
    this->settings = settings;
    this->size = this->settings->RegionSize;

    std::cout << "Initializing point lookup quadtree..." << std::endl;
    quadtree.InitializeQuadtree(this->size);

    // Now fill in all the quadtree files with the indexes of all the lines within the area.
    std::cout << "  Populating with " << lineStrips->lineStrips.size() << " line strips..." << std::endl;
    for (int i = 0; i < lineStrips->lineStrips.size(); i++)
    {
        if (this->settings->IsHighResolution)
        {
            AddPointsToQuadtree(i, lineStrips->lineStrips[i].points);
        }
        else
        {
            AddPointsToQuadtree(i, lineStrips->lineStrips[i].lowResPoints);
        }

        if (lineStrips->lineStrips.size() / 10 != 0 && (i % (lineStrips->lineStrips.size() / 10)) == 0)
        {
            std::cout << "  Processed line strip " << i << " of " << lineStrips->lineStrips.size() << std::endl;
        }
    }

    std::cout << "Quadtree initialized!" << std::endl;
}

// Same as the above but treats the index as a line.
double Rasterizer::GetLineDistanceSqd(Index idx, Point point)
{
    Point closestPoint = Point();
    if (this->settings->IsHighResolution)
    {
        Point& start = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx];
        Point& end = lineStrips->lineStrips[idx.stripIdx].points[idx.pointIdx + 1];
        ElevationComputer::GetClosestPointOnLine(point, start, end, &closestPoint);
    }
    else
    {
        LowResPoint& start = lineStrips->lineStrips[idx.stripIdx].lowResPoints[idx.pointIdx];
        LowResPoint& end = lineStrips->lineStrips[idx.stripIdx].lowResPoints[idx.pointIdx + 1];
        ElevationComputer::GetClosestPointOnLine(point, Point(start.x, start.y), Point(end.x, end.y), &closestPoint);
    }

    return pow(point.x - closestPoint.x, 2) + pow(point.y - closestPoint.y, 2);
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

double Rasterizer::ComputeElevation(Point point)
{
    sf::Vector2i quadSquare = GetQuadtreeSquare(point);

    // Loop forever as we are guaranteed to eventually find a point.
    int gridDistance = 1;
    std::vector<sf::Vector2i> searchQuads;

    int maxIterations = 90; // Hard stop to handle edge cases where there won't be edge lines for the computer to find.
    ElevationComputer elevationComputer = ElevationComputer(point);
    while (maxIterations > 0)
    {
        --maxIterations;
        searchQuads.clear();
        AddAreasToSearch(gridDistance, quadSquare, searchQuads);

        // Iterate through each search region and each element in each region, processing the line with the computer
        for (int k = 0; k < searchQuads.size(); k++)
        {
            int indexCount = (int)quadtree.ElementsInQuad(searchQuads[k]);
            for (size_t i = 0; i < indexCount; i++)
            {
                Index index = quadtree.GetIndexFromQuad(searchQuads[k], (int)i);
                LineStrip& lineStrip = lineStrips->lineStrips[index.stripIdx];

                if (this->settings->IsHighResolution)
                {
                    Point start = lineStrip.points[index.pointIdx];
                    Point end = lineStrip.points[index.pointIdx + 1];
                    elevationComputer.ProcessLine(start, end, lineStrip.elevation);
                }
                else
                {
                    LowResPoint& start = lineStrip.lowResPoints[index.pointIdx];
                    LowResPoint& end = lineStrip.lowResPoints[index.pointIdx + 1];
                    elevationComputer.ProcessLine(Point((double)start.x, (double)start.y), Point((double)end.x, (double)end.y), lineStrip.elevation);
                }
            }

            if (elevationComputer.HasSufficientData())
            {
                return elevationComputer.GetWeightedElevation();
            }
        }

        // Increment the grids we search.
        ++gridDistance;
    }

    return elevationComputer.GetWeightedElevation();
}

// Rasterizes a range of columns to improve perf.
void Rasterizer::RasterizeColumn(double leftOffset, double topOffset, double effectiveSize, int column, double** rasterStore, volatile bool* isRunning)
{
    for (int j = 0; j < size; j++)
    {
        double x = leftOffset + ((double)column / (double)size) * effectiveSize;
        double y = topOffset + ((double)j / (double)size) * effectiveSize;

        Point point(x, y);
        double elevation = ComputeElevation(point);
        (*rasterStore)[column + j * size] = elevation;
    }

	*isRunning = false;
}

void Rasterizer::Rasterize(double leftOffset, double topOffset, double effectiveSize, double** rasterStore)
{
    std::cout << "Region Rasterizing..." << std::endl;

    // Split apart rasterization across all cores - 1, or 7 if we can't find hardware cores.
    unsigned int hardwareThreads = std::thread::hardware_concurrency();
    int splitFactor = hardwareThreads < 3 ? 7 : hardwareThreads - 1;

	// Split apart rasterization evenly across the image so immediate results appear sooner
	// ... however also be smart enough to dedicate threads who complete their segment to new work.
    std::thread** threads = new std::thread*[splitFactor];
	int* currentThreadColumn = new int[splitFactor];
	volatile bool* threadsRunning = new bool[splitFactor];
	bool* rasterizedColumns = new bool[size];
	for (int i = 0; i < size; i++)
	{
		rasterizedColumns[i] = false;
	}
	
	int columnSplitAmount = size / splitFactor;
    for (int i = 0; i < splitFactor; i++)
    {
		int threadColumnOffset = i * columnSplitAmount;

		threadsRunning[i] = true;
		currentThreadColumn[i] = threadColumnOffset;
		rasterizedColumns[currentThreadColumn[i]] = true;
        threads[i] = new std::thread(&Rasterizer::RasterizeColumn, this, leftOffset, topOffset, effectiveSize, threadColumnOffset, rasterStore, &threadsRunning[i]);
    }

	bool columnsLeftToRasterize = true;
	int rasterizedColumnCount = 0;
	int lastPercent = -1;
	while (columnsLeftToRasterize)
	{
		columnsLeftToRasterize = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		for (int i = 0; i < splitFactor; i++)
		{
			// -1 == Used to represent dead threads (where there is only one column left and it already is being rasterized)
			if (!threadsRunning[i] && currentThreadColumn[i] != -1)
			{
				threads[i]->join();
				delete threads[i];

				// Compute and log overall status
				++rasterizedColumnCount;
				int rasterizationPercent = (int)(rasterizedColumnCount * 100 / size);
				if (rasterizationPercent % 10 == 0 && lastPercent != rasterizationPercent)
				{
					std::cout << "  Rasterization " << rasterizationPercent << "% complete." << std::endl;
					lastPercent = rasterizationPercent;
				}

				// Find first thread segment to join to help, starting from where we are
				int nextColumnToRasterize = currentThreadColumn[i] + 1;
				if (nextColumnToRasterize >= size || rasterizedColumns[nextColumnToRasterize])
				{
					// We can't just advance, find a new segment to pile on.
					nextColumnToRasterize = -1;
					for (int j = 0; j < splitFactor; j++)
					{
						int nextPossibleColumn = currentThreadColumn[j] + 1;
						if (nextPossibleColumn < size && !rasterizedColumns[nextPossibleColumn])
						{
							nextColumnToRasterize = nextPossibleColumn;
							break;
						}
					}
				}

				currentThreadColumn[i] = nextColumnToRasterize;
				if (nextColumnToRasterize != -1)
				{
					threadsRunning[i] = true;
					rasterizedColumns[nextColumnToRasterize] = true;
					threads[i] = new std::thread(&Rasterizer::RasterizeColumn, this, leftOffset, topOffset, effectiveSize, nextColumnToRasterize, rasterStore, &threadsRunning[i]);
				}
			}
			
			if (currentThreadColumn[i] != -1) // Use this instead of threadsRunning to avoid multithreading woes.
			{
				columnsLeftToRasterize = true;
			}
		}
	}


    delete[] threads;
	delete[] currentThreadColumn;
	delete[] threadsRunning;
	delete[] rasterizedColumns;
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

            bool onPoint = false;
            for (size_t k = 0; k < quadtree.ElementsInQuad(quadSquare); k++)
            {
                Index index = quadtree.GetIndexFromQuad(quadSquare, (int)k);

                if (this->settings->IsHighResolution)
                {
                    Point start = lineStrips->lineStrips[index.stripIdx].points[index.pointIdx];
                    Point end = lineStrips->lineStrips[index.stripIdx].points[index.pointIdx + 1];

                    if (std::pow(start.x - point.x, 2) + std::pow(start.y - point.y, 2) < wiggleDistSqd)
                    {
                        onPoint = true;
                        break;
                    }

                    if (std::pow(end.x - point.x, 2) + std::pow(end.y - point.y, 2) < wiggleDistSqd)
                    {
                        onPoint = true;
                        break;
                    }
                }
                else
                {
                    LowResPoint start = lineStrips->lineStrips[index.stripIdx].lowResPoints[index.pointIdx];
                    LowResPoint end = lineStrips->lineStrips[index.stripIdx].lowResPoints[index.pointIdx + 1];

                    if (std::pow(start.x - point.x, 2) + std::pow(start.y - point.y, 2) < wiggleDistSqd)
                    {
                        onPoint = true;
                        break;
                    }

                    if (std::pow(end.x - point.x, 2) + std::pow(end.y - point.y, 2) < wiggleDistSqd)
                    {
                        onPoint = true;
                        break;
                    }
                }
            }

            bool filled = false;
            if (!onPoint)
            {
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
            }

            (*rasterStore)[i + j * size] = onPoint ? 0.75 : (filled ? 1 : 0);
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