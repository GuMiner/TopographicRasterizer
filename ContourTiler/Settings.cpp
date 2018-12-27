#include <cctype>
#include <iostream>
#include <sstream>
#include "Settings.h"

// Setup defaults
Settings::Settings()
    : IsHighResolution(true), ElevationFeature("Elevation"), GeoJsonFiles(), OutputFolder("rasters"), RegionCount(10), RegionSize(800)
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
    for (size_t i = argument.length() - geoJsonKeyword.length(); i < geoJsonKeyword.length(); i++, counter++)
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

            if (equalsCaseInsensitive("--RegionCount", argv[i]) || equalsCaseInsensitive("-RegionCount", argv[i]))
            {
                if (i + 1 == argc)
                {
                    std::cout << "No region amount was found after '--RegionCount'!" << std::endl;
                    return false;
                }

                i++;
                std::istringstream inputStream(argv[i]);
                if (inputStream >> this->RegionCount ? false : true)
                {
                    std::cout << "Unable to parse the region count as an integer!" << std::endl;
                }

                if (this->RegionCount < 1)
                {
                    std::cout << "The region count must be at least 1! Found '" << this->RegionCount << "'." << std::endl;
                }

                parsedInput = true;
            }

            if (equalsCaseInsensitive("--RegionSize", argv[i]) || equalsCaseInsensitive("-RegionSize", argv[i]))
            {
                if (i + 1 == argc)
                {
                    std::cout << "No region size was found after '--RegionSize'!" << std::endl;
                    return false;
                }

                i++;
                std::istringstream inputStream(argv[i]);
                if (inputStream >> this->RegionSize ? false : true)
                {
                    std::cout << "Unable to parse the region size as an integer!" << std::endl;
                }

                if (this->RegionSize < 100) // Slightly arbitrary, but you can cut this down manually in post processing if you need to.
                {
                    std::cout << "The region size must be at least 100! Found '" << this->RegionSize << "'." << std::endl;
                }
                parsedInput = true;
            }

            if (equalsCaseInsensitive("--OutputFolder", argv[i]) || equalsCaseInsensitive("-OutputFolder", argv[i]))
            {
                if (i + 1 == argc)
                {
                    std::cout << "No output folder name was found after '--OutputFolder'!" << std::endl;
                    return false;
                }

                i++;
                this->OutputFolder = std::string(argv[i]);
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
    std::cout << "  All provided GeoJSON files are combined together and rasterized together." << std::endl;
    std::cout << std::endl;
    std::cout << "Input Format:" << std::endl;
    std::cout << "  Input files should be GeoJSON files with contours stored as MultiLineString objects." << std::endl;
    std::cout << "  By default, the elevation is assumed to be in the 'Elevation' feature, but the --Feature argument can override this." << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << " --Feature [Feature]: Specifies the input feature containing the elevation of each MultiLineString. Defaults to 'Elevation'." << std::endl;
    std::cout << " --RegionCount [Count]: Specifies the amount of tiling applied to the region. Defaults to 10 (which means 10x10 or 100 tiles are created)." << std::endl;
    std::cout << " --RegionSize [Size]: Specifies the size of each image created. Defaults to 800 (800x800 pixel images)." << std::endl;
    std::cout << "     This value should be around the size of your monitor, because the overview image is *also* rendered at this resolution. Use a higher region count if you need more detail." << std::endl;
    std::cout << " --OutputFolder [Folder]: Specifies the output folder rasterized images are placed. Defaults to 'rasters' (relative to the application). This folder must *not* exist." << std::endl;
    std::cout << " --LowResolution: Stores geometry data in 32-bit format. Useful for low-memory or large geometry regions. The default is high-resolution." << std::endl;
    std::cout << "Output Format:" << std::endl;
    std::cout << "  The rasterized, selected region is tiled into [RegionCount]x[RegionCount] images, each [RegionSize]x[RegionSize] in size." << std::endl;
    std::cout << "  These images are placed in subfolders in the [OutputFolder], where the sub folder name is the Y-coordinate and the image name the X-coordinate." << std::endl;
    std::cout << "  For example, with the default settings 100 1000x1000 images will be created, 10 images in the '0' to '9' folders within the [OutputFolder]." << std::endl;
    std::cout << "  These images represent height as 16-bit values in the red and grean fields of each image. Additional tooling shipped with this executable can convert these into greyscale (8 bit) heightmaps." << std::endl;
    std::cout << std::endl;
}