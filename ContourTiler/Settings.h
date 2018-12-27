#pragma once
#include <string>
#include <vector>

class Settings
{
    bool endsWithGeoJson(std::string argument);
    bool equalsCaseInsensitive(std::string a, std::string b);

public:
    Settings();
    bool ParseArguments(int argc, const char* argv[]);
    void OutputUsage();

    // Settings
    std::string ElevationFeature;
    int RegionCount;
    int RegionSize;
    std::string OutputFolder;
    bool IsHighResolution;
    std::vector<std::string> GeoJsonFiles;
};

