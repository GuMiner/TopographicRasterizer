#include <cctype>
#include <iostream>
#include "Settings.h"

// Setup defaults
Settings::Settings()
    : IsHighResolution(true), ElevationFeature("Elevation"), GeoJsonFiles()
{
}

bool Settings::endsWithGeoJson(std::string argument)
{
    std::string geoJsonKeyword(".geojson");

    if (argument.length() < geoJsonKeyword.length())
    {
        return false;
    }

    int counter = 0;
    for (int i = argument.length() - geoJsonKeyword.length(); i < geoJsonKeyword.length(); i++, counter++)
    {
        if (std::toupper(argument[i]) != std::toupper(geoJsonKeyword[counter]))
        {
            return false;
        }
    }

    return true;
}

bool Settings::equalsCaseInsensitive(std::string a, std::string b)
{
    if (a.length() != b.length())
    {
        return false;
    }

    for (int i = 0; i < a.length(); i++)
    {
        if (std::toupper(a[i]) != std::toupper(b[i]))
        {
            return false;
        }
    }

    return true;
}

bool Settings::ParseArguments(int argc, const char* argv[])
{
    if (argc == 1)
    {
        std::cout << "At least one geojson input file must be provided!" << std::endl;
        return false;
    }

    bool parsingGeoJson = true;
    for (int i = 1; i < argc; i++)
    {
        bool parsedInput = false;
        if (parsingGeoJson)
        {
            if (endsWithGeoJson(argv[i]))
            {
                this->GeoJsonFiles.push_back(std::string(argv[i]));
                parsedInput = true;
            }
            else
            {
                parsingGeoJson = false;
            }
        }

        if (!parsingGeoJson)
        {
            // See if this is an option, and if so, parse and continue
            if (equalsCaseInsensitive("--Feature", argv[i]) || equalsCaseInsensitive("-Feature", argv[i]))
            {
                if (i + 1 == argc)
                {
                    std::cout << "No feature name was found after '--Feature'!" << std::endl;
                    return false;
                }

                i++;
                this->ElevationFeature = std::string(argv[i]);
                parsedInput = true;
            }

            if (equalsCaseInsensitive("--LowResolution", argv[i]) || equalsCaseInsensitive("-LowResolution", argv[i]))
            {
                this->IsHighResolution = false;
                parsedInput = true;
            }
        }

        if (!parsedInput)
        {
            std::cout << "Did not recognize '" << argv[i] << "'!" << std::endl;
            return false;
        }
    }

    return true;
}

void Settings::OutputUsage()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "  ContourTiler.exe InputFile1.geojson InputeFile2.geojson ... [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "About:" << std::endl;
    std::cout << "  ContourTiler takes in a series of GeoJSON files, displays them, and manages rasterizing them into image heightmap files." << std::endl;
    std::cout << "  All provided GeoJSON files are combined together and displayed on the same image." << std::endl;
    std::cout << std::endl;
    std::cout << "Input Format:" << std::endl;
    std::cout << "  Input files should be GeoJSON files with contours stored as MultiLineString objects." << std::endl;
    std::cout << "  By default, the elevation is assumed to be in the 'Elevation' feature, but the --Feature argument can override this." << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << " --Feature [Feature]: Specifies the input feature containing the elevation of each MultiLineString." << std::endl;
    std::cout << " --LowResolution: Stores geometry data in 32-bit format. Useful for low-memory or large geometry regions." << std::endl;
}