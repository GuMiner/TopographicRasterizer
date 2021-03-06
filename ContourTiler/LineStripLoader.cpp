#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <limits>
#include <thread>
#include <map>
#include <set>
#include <nlohmann/json.hpp>
#include "LineStripLoader.h"

using json = nlohmann::json;

LineStripLoader::LineStripLoader()
{
}

bool LineStripLoader::Initialize(Settings* settings)
{
    std::map<int, int> elevationMap;
    lineStrips.clear();

    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    double minElevation = std::numeric_limits<double>::max();
    double maxElevation = std::numeric_limits<double>::lowest();

    long featureCount = 0;
    long lineSetCount = 0;
    long pointCount = 0;

    std::cout << "=== Validating Data ===" << std::endl;
    for (std::string geojsonFile : settings->GeoJsonFiles)
        {
            std::ifstream lsf(geojsonFile, std::ios::in | std::ios::binary);
            if (!lsf)
            {
                std::cout << "Could not open the file to read contours from: " << geojsonFile << std::endl;
                return false;
            }

            std::cout << "Loading GeoJSON file '" << geojsonFile << "' ..." << std::endl;
            json geoJson;
            lsf >> geoJson;
            std::cout << "Loaded GeoJSON file." << std::endl;

            std::cout << "Finding boundaries of the GeoJSON file..." << std::endl;
            double localMinX = std::numeric_limits<double>::max();
            double localMaxX = std::numeric_limits<double>::lowest();
            double localMinY = std::numeric_limits<double>::max();
            double localMaxY = std::numeric_limits<double>::lowest();
            double localMinElevation = std::numeric_limits<double>::max();
            double localMaxElevation = std::numeric_limits<double>::lowest();

            for (auto& feature : geoJson["features"])
            {
                if (feature["properties"].find(settings->ElevationFeature.c_str()) == feature["properties"].end())
                {
                    std::cout << "Could not find the property '" << settings->ElevationFeature << "' in the list of known properties for a feature!" << std::endl;
                    return false;
                }

                double elevation = 0.0;
                if (feature["properties"][settings->ElevationFeature.c_str()].is_number_integer())
                {
                    elevation = int(feature["properties"][settings->ElevationFeature.c_str()].get<int>());
                }
                else if (feature["properties"][settings->ElevationFeature.c_str()].is_number_float())
                {
                    elevation = feature["properties"][settings->ElevationFeature.c_str()].get<double>();
                }
                else
                {
                    std::cout << "The given elevation property was not an integer or a floating point value, but a '" <<
                        feature["properties"][settings->ElevationFeature.c_str()].type_name() << "'. Only these two value types are supported." << std::endl;
                    return false;
                }

                // Check to see if elevation redefines the boundaries.
                localMinElevation = std::min(elevation, localMinElevation);
                localMaxElevation = std::max(elevation, localMaxElevation);

                // Check to see if the countours re-define the boundaries.
                for (auto& lineSet : feature["geometry"]["coordinates"])
                {
                    for (auto& point : lineSet)
                    {
                        double x = point[0];
                        double y = point[1];
                        localMinX = std::min(x, localMinX);
                        localMinY = std::min(y, localMinY);
                        localMaxX = std::max(x, localMaxX);
                        localMaxY = std::max(y, localMaxY);

                        ++pointCount;
                    }

                    ++lineSetCount;
                }

                ++featureCount;
            }

            std::cout << "Boundaries found of the file!" << std::endl;
            std::cout << "  X: [" << localMinX << ", " << localMaxX << "], Y: [" << localMinY << ", " << localMaxY << "], Elevation: [" << localMinElevation << "," << localMaxElevation << "]" << std::endl;

            minX = std::min(localMinX, minX);
            minY = std::min(localMinY, minY);
            minElevation = std::min(localMinElevation, minElevation);
            maxX = std::max(localMaxX, maxX);
            maxY = std::max(localMaxY, maxY);
            maxElevation = std::max(localMaxElevation, maxElevation);

            std::cout << "  Global boundaries (all files) updated to:" << std::endl;
            std::cout << "    X: [" << minX << ", " << maxX << "], Y: [" << minY << ", " << maxY << "], Elevation: [" << minElevation << "," << maxElevation << "]" << std::endl;
        }

    std::cout << std::endl;
    std::cout << "Global boundaries (all files):" << std::endl;
    std::cout << "  X: [" << minX << ", " << maxX << "], Y: [" << minY << ", " << maxY << "], Elevation: [" << minElevation << "," << maxElevation << "]" << std::endl;
    std::cout << "Statistics: " << std::endl;
    std::cout << "  Features: " << featureCount << ". Line sets: " << lineSetCount << ". Points: " << pointCount << "." << std::endl;
    std::cout << std::endl;

    long parsedPoints = 0;


    std::cout << "=== Importing Data ===" << std::endl;
    std::set<double> uniqueElevations = std::set<double>();
    for (std::string geojsonFile : settings->GeoJsonFiles)
    {
        std::ifstream lsf(geojsonFile, std::ios::in | std::ios::binary);
        std::cout << "Loading GeoJSON file " << geojsonFile << "..." << std::endl;
        json geoJson;
        lsf >> geoJson;
        std::cout << "Loaded GeoJSON file." << std::endl;

        std::cout << "Loading and normalizing features..." << std::endl;
        for (auto& feature : geoJson["features"])
        {
            double elevation = 0.0;
            if (feature["properties"][settings->ElevationFeature.c_str()].is_number_integer())
            {
                elevation = int(feature["properties"][settings->ElevationFeature.c_str()].get<int>());
            }
            else // if (feature["properties"][settings->ElevationFeature.c_str()].is_number_float())
            {
                elevation = feature["properties"][settings->ElevationFeature.c_str()].get<double>();
            }

            for (auto& lineSet : feature["geometry"]["coordinates"])
            {
                lineStrips.push_back(LineStrip());
                size_t i = lineStrips.size() - 1;

                lineStrips[i].elevation = (elevation - minElevation) / (maxElevation - minElevation);
                uniqueElevations.emplace(lineStrips[i].elevation);

                lineStrips[i].points.clear();
                lineStrips[i].lowResPoints.clear();
                for (auto& point : lineSet)
                {
                    const double x = point[0];
                    const double y = point[1];
                    Point parsedPoint;
                    parsedPoint.x = (x - minX) / (maxX - minX);
                    parsedPoint.y = 1.0 - ((y - minY) / (maxY - minY));

                    if (settings->IsHighResolution)
                    {
                        lineStrips[i].points.push_back(parsedPoint);
                    }
                    else
                    {
                        LowResPoint lrPoint;
                        lrPoint.x = (float)parsedPoint.x;
                        lrPoint.y = (float)parsedPoint.y;
                        lineStrips[i].lowResPoints.push_back(lrPoint);
                    }

                    ++parsedPoints;
                    if (parsedPoints % (pointCount / 10) == 0)
                    {
                        std::cout << "  Point " << parsedPoints << " of " << pointCount << " loaded." << std::endl;
                    }
                }
            }
        }
    }

    // Useful for runtime diagnosis
    std::cout << "Found " << uniqueElevations.size() << " unique elevations in the provided inputs." << std::endl;
    return true;
}

LineStripLoader::~LineStripLoader()
{
}