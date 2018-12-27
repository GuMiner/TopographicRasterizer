#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <SFML\System.hpp>
#include "Index.h"

class Quadtree
{
    int size;
    std::vector<std::vector<Index>> quadtree;

public:
    Quadtree();

    void InitializeQuadtree(int size);
    void AddToIndex(sf::Vector2i quadtreePos, Index index);
    size_t QuadSize(sf::Vector2i quadtreePos) const;
    Index GetIndexFromQuad(sf::Vector2i quadtreePos, int offset) const;
};

