#pragma once
#include <SFML\System.hpp>
#include <SFML\Graphics.hpp>
#include <vector>
#include <future>
#include <string>
#include "ColorMapper.h"
#include "LineStripLoader.h"
#include "Rasterizer.h"
#include "Settings.h"

// Handles startup and the base graphics rendering loop.
class ContourTiler
{
    Settings* settings;

    bool renderColors;
    bool renderContours;

    bool rerender;
    sf::Vector2i mouseStart;
    sf::Vector2i mousePos;
    sf::RectangleShape zoomShape;

    double leftOffset;
    double topOffset;
    double effectiveSize;

    double* rasterizationBuffer;
    Rasterizer rasterizer;
    std::future<void> renderingThread;
    bool isRendering;
    sf::Time rasterStartTime;
    LineStripLoader lineStripLoader;

    double* linesBuffer;

    bool isZoomMode;
    ColorMapper colorMapper;
    sf::Time lastUpdateTime;
    
    sf::Texture overallTexture;
    sf::Sprite overallSprite;
    void SetupGraphicsElements();
    void FillOverallTexture();
    void UpdateTextureFromBuffer();

    int regionX, regionY;
    void ZoomToRegion(int x, int y);
    bool isBulkProcessing;
    sf::Time regionStartTime;

    bool outputHelp;
    void OutputDisplayHelp();

    // Handles GUI-based events, such as closing the application, resizing the window, etc.
    void HandleEvents(sf::RenderWindow& window, bool& alive);

    // Renders the scene.
    void Render(sf::RenderWindow& window, sf::Time elapsedTime);

public:
    // Initializes the ContourTiler
    ContourTiler();
    virtual ~ContourTiler();

    // Runs the game loop.
    void Run(Settings* settings);
};