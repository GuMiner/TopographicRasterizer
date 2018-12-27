#include <array>
#include <algorithm>
#include <cmath>
#include <direct.h>
#include <iostream>
#include <future>
#include <limits>
#include <string>
#include <sstream>
#include <thread>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <stb/stb_image_write.h>
#include "ContourTiler.h"
#include "Settings.h"

#ifndef _DEBUG
    #pragma comment(lib, "../lib/sfml-system")
    #pragma comment(lib, "../lib/sfml-window")
    #pragma comment(lib, "../lib/sfml-graphics")
#else
    #pragma comment(lib, "../lib/sfml-system-d")
    #pragma comment(lib, "../lib/sfml-window-d")
    #pragma comment(lib, "../lib/sfml-graphics-d")
#endif

const char* RasterFolder = "madison_rasters";

ContourTiler::ContourTiler()
    : lineStripLoader(), size(1000), regionSize(10), rasterizer(&lineStripLoader, size), minElevation(0), maxElevation(1), rasterizationBuffer(new double[size * size]), linesBuffer(new double[size * size]), coverBuffer(new bool[size * size]),
      leftOffset((double)0.0), topOffset((double)0.0), effectiveSize((double)1.0), mouseStart(-1, -1), mousePos(-1, -1), isRendering(false), isZoomMode(true),
      rerender(false), isBulkProcessing(false), regionX(0), regionY(0)
{ }

ContourTiler::~ContourTiler()
{
    delete[] rasterizationBuffer;
    delete[] linesBuffer;
}

void ContourTiler::OutputDisplayHelp()
{
    std::cout << "The graphical display supports the following commands:" << std::endl;
    std::cout << "  Display" << std::endl;
    std::cout << "    Left-click and hold/drag to select an area to zoom to. Release to zoom in." << std::endl;
    std::cout << "    Right-click and release to zoom out." << std::endl;
    std::cout << "    R: Resets the view to the default region" << std::endl;
    std::cout << "    L: Enables or disables rendering contour lines." << std::endl;
    std::cout << "    C: Enables or disables rendering a color spectrum overlay." << std::endl;
    std::cout << "  Export" << std::endl;
    std::cout << "    P: Starts bulk processing, dividing up the image into regions and rendering heightmaps" << std::endl;

}

void ContourTiler::HandleEvents(sf::RenderWindow& window, bool& alive)
{
    // Handle all events.
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            alive = false;
        }
        else if (event.type == sf::Event::KeyReleased)
        {
            if (event.key.code == sf::Keyboard::R)
            {
                // Reset
                topOffset = 0.0f;
                leftOffset = 0.0f;
                effectiveSize = 1.0f;
                rerender = true;
                std::cout << "Reset display " << std::endl;
            }
            else if (event.key.code == sf::Keyboard::L)
            {
                // Contour lines
                this->renderContours = !this->renderContours;
                std::cout << "Toggled contour rendering: " << (this->renderContours ? "on" : "off") << std::endl;
            }
            else if (event.key.code == sf::Keyboard::C)
            {
                // Colorize (true/false)
                this->renderColors = !this->renderColors;
                std::cout << "Toggled color rendering: " << (this->renderColors ? "on" : "off") << std::endl;
            }
            else if (event.key.code == sf::Keyboard::P)
            {
                // Bulk processing divides the area into 3-ft resolution areas (regionSize x regionSize or 70x70) all 1000x1000 pixels.
                std::cout << "Starting bulk processing mode." << std::endl;
                
                // Point of no return -- this continues until done.
                ZoomToRegion(regionX, regionY);
                isBulkProcessing = true;
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                // Zoom-in preparation
                mouseStart = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                mousePos = mouseStart;
            }
            else if (event.mouseButton.button == sf::Mouse::Right)
            {
                // Zoom-out.
                topOffset -= effectiveSize * 0.50f;
                leftOffset -= effectiveSize * 0.50f;
                effectiveSize *= 3.0f;

                // Clamp.
                if (topOffset < 0)
                {
                    topOffset = 0;
                }
                if (leftOffset < 0)
                {
                    leftOffset = 0;
                }
                if (topOffset + effectiveSize > 1)
                {
                    effectiveSize = 1 - topOffset;
                }
                if (leftOffset + effectiveSize > 1)
                {
                    effectiveSize = 1 - leftOffset;
                }

                std::cout << "Zooming out to [" << leftOffset << ", " << topOffset << ", " << effectiveSize << ", " << effectiveSize << "]" << std::endl;
                sf::sleep(sf::milliseconds(500));
                rerender = true;
            }
        }
        else if (event.type == sf::Event::MouseMoved)
        {
            // Update for the zoom rectangle
            mousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
        }
        else if (event.type == sf::Event::MouseButtonReleased)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                int xNew = event.mouseButton.x;
                int yNew = event.mouseButton.y;
                if (xNew > mouseStart.x && yNew > mouseStart.y)
                {
                    // We have a valid zoom-in. Determine the new bounding box. However, we want a proper scaling factor.
                    double scalingFactor = std::min(((double)(xNew - mouseStart.x) / (double)size), ((double)(yNew - mouseStart.y / (double)size)));

                    leftOffset += ((double)mouseStart.x / (double)size) * effectiveSize;
                    topOffset += ((double)mouseStart.y / (double)size) * effectiveSize;
                    effectiveSize = scalingFactor * effectiveSize;
                    std::cout << "Zooming in to [" << leftOffset << ", " << topOffset << ", " << effectiveSize << ", " << effectiveSize << "]" << std::endl;
                    rerender = true;
                }

                mouseStart = sf::Vector2i(-1, -1);
            }
        }
    }
}

void ContourTiler::ZoomToRegion(int x, int y)
{
    double viewSize = 1.0 / (double)regionSize;
    leftOffset = (double)x * viewSize;
    topOffset = (double)y * viewSize;
    effectiveSize = viewSize;
    rerender = true;
}

void ContourTiler::SetupGraphicsElements()
{
    // Sets up the background and zoom shape
    overallTexture.create(size, size);
    overallTexture.setRepeated(false);
    overallTexture.setSmooth(false);

    overallSprite.setTexture(overallTexture);

    zoomShape.setOutlineColor(sf::Color::Green);
    zoomShape.setOutlineThickness(1);
    zoomShape.setFillColor(sf::Color::Transparent);
    
    rerender = true;
}

void ContourTiler::FillOverallTexture()
{
    // Rasterize
    rasterizer.Rasterize(leftOffset, topOffset, effectiveSize, &rasterizationBuffer, minElevation, maxElevation);
    rasterizer.LineRaster(leftOffset, topOffset, effectiveSize, &linesBuffer);

    UpdateTextureFromBuffer();
}

void ContourTiler::ClearCoverPane()
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            coverBuffer[i + j*size] = false;
        }
    }
}

void ContourTiler::UpdateTextureFromBuffer()
{
    // Copy over to the image with an appropriate color mapping.
    sf::Uint8* pixels = new sf::Uint8[size * size * 4]; // * 4 because pixels have 4 components (RGBA)
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            double elevation = rasterizationBuffer[i + j * size];
            int pixelIdx = (i + j * size) * 4;

            if (this->renderColors)
            {
                colorMapper.MapColor(elevation, &pixels[pixelIdx], &pixels[pixelIdx + 1], &pixels[pixelIdx + 2]);
            }
            else
            {
                int z = (int)(elevation * 255);

                pixels[pixelIdx] = z;
                pixels[pixelIdx + 1] = z;
                pixels[pixelIdx + 2] = z;
            }

            // Lines buffer modification, only if applicable.
            if (this->renderContours && linesBuffer[i + j * size] > 0.5)
            {
                pixels[pixelIdx] = std::min(255, pixels[pixelIdx] + 50);
                pixels[pixelIdx + 1] = std::min(255, pixels[pixelIdx] + 50);
            }

            pixels[pixelIdx + 3] = 255;
        }
    }

    overallTexture.update(pixels);
    delete[] pixels;
}

void ContourTiler::Render(sf::RenderWindow& window, sf::Time elapsedTime)
{
    // Rerender as needed on a separate thread.
    if (rerender && !isRendering)
    {
        ClearCoverPane();
        isRendering = true;
        rasterStartTime = elapsedTime;
        renderingThread = std::async(std::launch::async, &ContourTiler::FillOverallTexture, this);
    }
    else if (isRendering)
    {
        std::future_status status = renderingThread.wait_until(std::chrono::system_clock::now());
        if (status == std::future_status::ready)
        {
            rerender = false;
            isRendering = false;
            std::cout << "Raster time: " << (elapsedTime - rasterStartTime).asSeconds() << " s." << std::endl;

            if (isBulkProcessing)
            {
                // Create a new folder for the current region.
                if (regionX == 0)
                {
                    std::stringstream folder;
                    folder << ".\\" << RasterFolder << "\\" << regionY;
                    _mkdir(folder.str().c_str());
                    std::cout << "Making directory " << folder.str().c_str() << std::endl;
                }

                // Save out our current data
                std::stringstream file;
                file << RasterFolder << "/" << regionY << "/" << regionX << ".png";

                unsigned char* data = new unsigned char[size * size * 4];
                for (int i = 0; i < size; i++)
                {
                    for (int j = 0; j < size; j++)
                    {
                        // RGBA order
                        int scaledVersion = std::min((int)(rasterizationBuffer[i + j * size] * (65536)), 65535);
                        // RED == upper 8 bytes.
                        // GREEN == lower 8 bytes.

                        data[(i + j * size) * 4] = (unsigned char)(scaledVersion & 0x00FF);
                        data[(i + j * size) * 4 + 1] = (unsigned char)((scaledVersion & 0xFF00) >> 8);
                        data[(i + j * size) * 4 + 2] = 255;
                        data[(i + j * size) * 4 + 3] = 255;
                    }
                }

                const int RGBA = 4;
                std::cout << file.str().c_str() << std::endl;
                int result = stbi_write_png(file.str().c_str(), size, size, RGBA, &data[0], size * 4 * sizeof(unsigned char));
                if (result != 0)
                {
                    std::cout << "Failed to write a file: " << result << " for raster " << regionX << ", " << regionY << std::endl;
                }
                
                std::cout << "Wrote the file " << regionX << ", " << regionY << std::endl;
                delete[] data;

                // Move to the next region.
                regionX++;
                if (regionX == regionSize)
                {
                    regionX = 0;
                    regionY++;
                }

                if (regionY != regionSize) // Continue;
                {
                    ZoomToRegion(regionX, regionY);
                }
                else
                {
                    isBulkProcessing = false;
                }
            }
        }
    }

    // Update the texture at a reasonable but not too fast pace.
    if (elapsedTime - lastUpdateTime > sf::milliseconds(333))
    {
        lastUpdateTime = elapsedTime;
        UpdateTextureFromBuffer();
    }

    // Draw the raster result and our zoom box if applicable.
    window.clear(sf::Color::Blue);
    window.draw(overallSprite);

    if (mouseStart.x != -1)
    {
        zoomShape.setPosition(sf::Vector2f((float)mouseStart.x, (float)mouseStart.y));
        float minDiff = std::min((float)(mousePos.x - mouseStart.x), (float)(mousePos.y - mouseStart.y));
        zoomShape.setSize(sf::Vector2f(minDiff, minDiff));
        window.draw(zoomShape);
    }
}

void ContourTiler::Run(Settings* settings)
{
    // == Load data ==
    // Load our data file.
    if (!lineStripLoader.Initialize(settings))
    {
        std::cout << "Could not parse the input GeoJSON files!" << std::endl;
        return;
    }

    std::cout << std::endl;
    std::cout << "==Initializing Environment==" << std::endl;
    rasterizer.Setup(settings);

    // == Setup graphics ==
    // 24 depth bits, 8 stencil bits, 8x AA, major version 4.
    sf::ContextSettings contextSettings = sf::ContextSettings(24, 8, 8, 4, 0);

    sf::Uint32 style =  sf::Style::Titlebar | sf::Style::Close;
    sf::RenderWindow window(sf::VideoMode(size, size), "Contour Tiler", style, contextSettings);
    window.setFramerateLimit(60);

    this->SetupGraphicsElements();

    // == Start the main loop ==
    bool alive = true;
    sf::Clock timer;
    this->lastUpdateTime = timer.getElapsedTime();
    while (alive)
    {
        HandleEvents(window, alive);
        Render(window, timer.getElapsedTime());
        window.display();
    }
}

// Performs the graphical interpolation and tiling of contours.
int main(int argc, const char* argv[])
{
    std::cout << "ContourTiler" << std::endl;
    Settings settings;
    if (settings.ParseArguments(argc, argv))
    {
        std::unique_ptr<ContourTiler> contourTiler(new ContourTiler());
        contourTiler->Run(&settings);
        std::cout << "Done." << std::endl;
        return 0;
    }
    else
    {
        std::cout << "Unable to parse the input arguments!" << std::endl;
        settings.OutputUsage();
        return 1;
    }
}