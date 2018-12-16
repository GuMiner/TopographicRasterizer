#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <SFML\System.hpp>
#include "Definitions.h"

class Quadtree
{
    int size;
    std::vector<std::vector<Index>> quadtree;

    // std::vector<std::vector<std::ofstream*>> quadtreeFiles;
    // std::vector<std::vector<long>> quadtreeSizes;

    // std::queue<sf::Vector2i> openFileQueue;
    // void OpenFile(sf::Vector2i index);

    // std::string GetQuadtreeFilename(int x, int y);
    // std::string quadtreeFolderName;
public:
    Quadtree(int size);

    void InitializeQuadtree();
    void AddToIndex(sf::Vector2i quadtreePos, Index index);
    size_t QuadSize(sf::Vector2i quadtreePos) const;
    Index GetIndexFromQuad(sf::Vector2i quadtreePos, int offset) const;
    // void CreateQuadtreeFiles(std::string quadtreeFolder);
    // void CloseFiles();

    ~Quadtree();
};

