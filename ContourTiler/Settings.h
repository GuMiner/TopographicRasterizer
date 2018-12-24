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
    bool IsHighResolution;
    std::string ElevationFeature;
    std::string ExclusionFile;
    std::vector<std::string> GeoJsonFiles;
};

