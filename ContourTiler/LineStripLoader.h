#pragma once
#include <string>
#include "Definitions.h"

class LineStripLoader
{
public:
    LineStripLoader();

    std::vector<LineStrip> lineStrips;
    bool Initialize(std::string lineStripFilename);

    virtual ~LineStripLoader();
};

