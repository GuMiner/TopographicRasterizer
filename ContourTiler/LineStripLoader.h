#pragma once
#include <string>
#include "LineStrip.h"
#include "Settings.h"

class LineStripLoader
{
public:
    LineStripLoader();

    std::vector<LineStrip> lineStrips;
    bool Initialize(Settings* settings);

    virtual ~LineStripLoader();
};

