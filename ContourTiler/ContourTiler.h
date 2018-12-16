#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <future>
#include <string>
#include "ColorMapper.h"
#include "Definitions.h"
#include "LineStripLoader.h"
#include "Rasterizer.h"

struct ViewOptions
{
    bool colorize, rescale, lines;

    ViewOptions()
        : colorize(false), rescale(false), lines(true)
    {
    }
};

// Handles startup and the base graphics rendering loop.
class ContourTiler
{
    int size;
    int regionSize;

    bool rerender;
    sf::Vector2i mouseStart;
    sf::Vector2i mousePos;
    sf::RectangleShape zoomShape;

    sf::RectangleShape exclusionShape;
    bool hideExclusionShape;

    decimal leftOffset;
    decimal topOffset;
    decimal effectiveSize;

    double minElevation, maxElevation;
    double* rasterizationBuffer;
    Rasterizer rasterizer;
    std::future<void> renderingThread;
    bool isRendering;
    sf::Time rasterStartTime;
    LineStripLoader lineStripLoader;
    QuadExclusions quadExclusions;

    double* linesBuffer;
    bool* coverBuffer;

    bool isZoomMode;
    ViewOptions viewOptions;
    ColorMapper colorMapper;
    sf::Time lastUpdateTime;
    
    sf::Texture overallTexture;
    sf::Sprite overallSprite;
    void SetupGraphicsElements();
    void FillOverallTexture();
    void UpdateTextureFromBuffer();
    
    void ClearCoverPane();

    int regionX, regionY;
    void ZoomToRegion(int x, int y);
    bool isBulkProcessing;
    sf::Time regionStartTime;

    // Handles GUI-based events, such as closing the application, resizing the window, etc.
    void HandleEvents(sf::RenderWindow& window, bool& alive);

    // Renders the scene.
    void Render(sf::RenderWindow& window, sf::Time elapsedTime);

public:
    // Initializes the ContourTiler
    ContourTiler();
    virtual ~ContourTiler();

    // Runs the game loop.
    void Run();
};