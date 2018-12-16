#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <limits>
#include <thread>
#include <map>
#include <nlohmann/json.hpp>
#include "LineStripLoader.h"

using json = nlohmann::json;

LineStripLoader::LineStripLoader()
{

}

// https://stackoverflow.com/questions/874134/find-if-string-ends-with-another-string-in-c
bool hasEnding(std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length())
    {
        return fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0;
    }

    return false;
}

bool LineStripLoader::Initialize(std::string lineStripFilename)
{
    std::map<int, int> elevationMap;
    lineStrips.clear();
    if (hasEnding(lineStripFilename, ".bin"))
    {
        return false;
        // std::ifstream lineStripFile(lineStripFilename, std::ios::in | std::ios::binary);
        // if (!lineStripFile)
        // {
        //     std::cout << "Could not open the file to read contours from!" << std::endl;
        //     return false;
        // }
        // 
        // int lineStripCount;
        // lineStripFile.read((char*)&lineStripCount, sizeof(int));
        // std::cout << "Line strips: " << lineStripCount << std::endl;
        // 
        // // lineStripCount = 300000; // Override to limit load time for rasterization changes.
        // 
        // // Reported settings.
        // double xMinRep, xMaxRep, yMinRep, yMaxRep, minERep, maxERep;
        // lineStripFile.read((char*)&xMinRep, sizeof(double));
        // lineStripFile.read((char*)&xMaxRep, sizeof(double));
        // lineStripFile.read((char*)&yMinRep, sizeof(double));
        // lineStripFile.read((char*)&yMaxRep, sizeof(double));
        // lineStripFile.read((char*)&minERep, sizeof(double));
        // lineStripFile.read((char*)&maxERep, sizeof(double));
        // 
        // std::cout.precision(10);
        // std::cout << "Limits reported to be [" << xMinRep << ", " << xMaxRep << "; " << yMinRep << ", " << yMaxRep << "; " << minERep << ", " << maxERep << "]." << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // 
        // // Line strips now follow sequentially with the following format:
        // //   double elevation;
        // //   int pointCount;
        // //   Point points[pointCount];
        // lineStrips.reserve(lineStripCount);
        // long points = 0;
        // 
        // for (int i = 0; i < lineStripCount; i++)
        // {
        //     lineStrips.push_back(LineStrip());
        // 
        //     double elevation;
        //     lineStripFile.read((char*)&elevation, sizeof(double));
        //     if (elevationMap.find((int)elevation) == elevationMap.end())
        //     {
        //         int size = (int)elevationMap.size();
        //         lineStrips[i].elevationId = size;
        //         elevationMap[(int)elevation] = size;
        //     }
        //     else
        //     {
        //         lineStrips[i].elevationId = elevationMap[(int)elevation];
        //     }
        // 
        //     lineStrips[i].elevation = (elevation - Constant::ZMin) / (Constant::ZMax - Constant::ZMin);
        // 
        //     int pointCount;
        //     lineStripFile.read((char*)&pointCount, sizeof(int));
        // 
        //     int actualPointCount = pointCount;
        //     if (pointCount > 100)
        //     {
        //         // pointCount = 100; // Override to speed up loading yet keep a reasonable overview.
        //     }
        // 
        //     lineStrips[i].points.reserve(pointCount);
        // 
        //     // Read in all the data, but don't process the bulk of it for quick exclusion tests.
        //     double* allPoints = new double[actualPointCount * 2];
        //     lineStripFile.read((char*)allPoints, sizeof(double) * 2 * actualPointCount);
        // 
        //     for (int j = 0; j < pointCount; j++)
        //     {
        //         ++points;
        //         if (points % 1000000 == 0)
        //         {
        //             std::cout << "Read in point " << points << ". (" << (float)points / 1.5e7f << "%)" << std::endl;
        //         }
        // 
        //         Point point;
        //         point.x = (decimal)((allPoints[j * 2] - Constant::XMin) / (Constant::XMax - Constant::XMin));
        //         point.y = (decimal)((allPoints[j * 2 + 1] - Constant::YMin) / (Constant::YMax - Constant::YMin));
        //         lineStrips[i].points.push_back(point);
        //     }
        // 
        //     delete[] allPoints;
        //     if (i % 10000 == 0)
        //     {
        //         std::cout << "Read in line strip " << i << std::endl;
        //     }
        // }
    }
    else
    {
        // Assumed to be GeoJSON
        std::vector<std::string> geojsonFiles;
        // geojsonFiles.push_back
            // geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\Hawaii\\geojson\\KahOne.geojson");
            // geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\Hawaii\\geojson\\KauOne.geojson");
            // geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\Hawaii\\geojson\\MauOne.geojson");
            // geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\Hawaii\\geojson\\MolOne.geojson");
            // geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\Hawaii\\geojson\\OahOne.geojson");
            // geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\Hawaii\\geojson\\LanOne.geojson");
            // geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\Hawaii\\geojson\\NiiOne.geojson");
            // geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\Hawaii\\geojson\\HawOne.geojson");
            geojsonFiles.push_back("C:\\Users\\Gustave\\Desktop\\madison\\clippedMadison.geojson");

        for (std::string geojsonFile : geojsonFiles)
        {
            std::ifstream lsf(geojsonFile, std::ios::in | std::ios::binary);
            if (!lsf)
            {
                std::cout << "Could not open the file to read contours from: " << geojsonFile << std::endl;
                return false;
            }

            std::cout << "Loading GeoJSON file" << geojsonFile << "." << std::endl;
            json geoJson;
            lsf >> geoJson;
            std::cout << "Loaded GeoJSON file." << std::endl;

            int minElevation = 1000000;
            int maxElevation = -1000000;
            double maxX = -89.5519742238397356; //  std::numeric_limits<double>::max();
            double minY = 42.8609070451122562; // std::numeric_limits<double>::max();
            double minX = -89.1396831709526651; // std::numeric_limits<double>::min();
            double maxY = 43.2293901983112434; // std::numeric_limits<double>::min();
            // for (auto& feature : geoJson["features"])
            // {
            //     for (auto& lineSet : feature["geometry"]["coordinates"])
            //     {
            //         for (auto& point : lineSet)
            //         {
            
                // - 89.5519742238397356, 42.8609070451122562 : -89.1396831709526651, 43.2293901983112434

            //             // std::cout << "Loading point: " << point.dump() << std::endl;
            //             double x = point[0];
            //             double y = point[1];
            //             minX = std::min(x, minX);
            //             minY = std::min(y, minY);
            //             maxX = std::max(x, maxX);
            //             maxY = std::max(y, maxY);
            //         }
            //     }
            // }
            // std::cout << minX << " " << minY << " " << maxX << " " << maxY << std::endl;

            long featureCount = 0;
            long pointCount = 0;
            std::cout << "Processing features..." << std::endl;
            for (auto& feature : geoJson["features"])
            {
                int elevation = feature["properties"]["ContourEle"]; //["CONTOUR"];
                minElevation = std::min(elevation, minElevation);
                maxElevation = std::max(elevation, maxElevation);
                // std::cout << elevation << '\n';

                ++featureCount;
                if (featureCount % 1000 == 0)
                {
                    std::cout << "(Count: " << featureCount << " " << elevation << ") " << std::endl;
                }

                // std::cout << "Processing line set..." << std::endl;
                for (auto& lineSet : feature["geometry"]["coordinates"])
                {
                    // std::cout << "Processing line in set..." << std::endl;
                    lineStrips.push_back(LineStrip());
                    int i = lineStrips.size() - 1;

                    // Save elevation and elevationId.
                    if (elevationMap.find(elevation) == elevationMap.end())
                    {
                        int size = (int)elevationMap.size();
                        lineStrips[i].elevationId = size;
                        elevationMap[elevation] = size;
                    }
                    else
                    {
                        lineStrips[i].elevationId = elevationMap[(int)elevation];
                    }

                    lineStrips[i].elevation = ((double)elevation - Constant::ZMin) / (Constant::ZMax - Constant::ZMin);

                    lineStrips[i].points.clear();
                    for (auto& point : lineSet)
                    {
                        // std::cout << "Loading point: " << point.dump() << std::endl;
                        double x = point[0];
                        double y = point[1];
                        Point parsedPoint;
                        parsedPoint.x = (decimal)((x - minX) / (maxX - minX));
                        parsedPoint.y = (decimal)((y - minY) / (maxY - minY));

                        if (parsedPoint.x < 0 || parsedPoint.x > 1)
                        {
                            std::cout << point.dump() << " " << elevation << "-" << parsedPoint.x << "+" << parsedPoint.y << std::endl;
                            return false;
                        }
                        else if (parsedPoint.y < 0 || parsedPoint.y > 1)
                        {
                            std::cout << point.dump() << " " << elevation << "-" << parsedPoint.x << "+" << parsedPoint.y << std::endl;
                            return false;
                        }

                        lineStrips[i].points.push_back(parsedPoint);

                        ++pointCount;
                        if (pointCount % 10000 == 0)
                        {
                            std::cout << "(Count: " << pointCount << " " << elevation << ") " << std::endl;
                        }
                    }
                }
            }

            std::cout << "Feature Count: " << featureCount << std::endl;
            std::cout << "Elevation: " << minElevation << " " << maxElevation << std::endl;
            std::cout << "Bounds: " << minX << " " << minY << " " << maxX << " " << maxY << std::endl;
        }
    }

    return true;
}

LineStripLoader::~LineStripLoader()
{
}