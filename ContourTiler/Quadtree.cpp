#include <sstream>
#include <direct.h>
#include <iostream>
#include "Quadtree.h"

Quadtree::Quadtree()
{ }

void Quadtree::InitializeQuadtree(int size)
{
    this->size = size;
    quadtree.clear();
    for (int i = 0; i < size * size; i++)
    {
        quadtree.push_back(std::vector<Index>());
    }
}

void Quadtree::AddToIndex(sf::Vector2i quadtreePos, Index index)
{
    quadtree[quadtreePos.x + size * quadtreePos.y].push_back(index);
}

size_t Quadtree::ElementsInQuad(sf::Vector2i quadtreePos) const
{
    return quadtree[quadtreePos.x + size * quadtreePos.y].size();
}

Index Quadtree::GetIndexFromQuad(sf::Vector2i quadtreePos, int offset) const
{
    return quadtree[quadtreePos.x + size * quadtreePos.y][offset];
}
