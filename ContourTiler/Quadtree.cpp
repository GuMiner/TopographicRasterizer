#include <sstream>
#include <direct.h>
#include <iostream>
#include "Quadtree.h"


Quadtree::Quadtree(int size) : size(size)
{ }

void Quadtree::InitializeQuadtree()
{
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

size_t Quadtree::QuadSize(sf::Vector2i quadtreePos) const
{
    return quadtree[quadtreePos.x + size * quadtreePos.y].size();
}

Index Quadtree::GetIndexFromQuad(sf::Vector2i quadtreePos, int offset) const
{
    return quadtree[quadtreePos.x + size * quadtreePos.y][offset];
}

/*
void Quadtree::OpenFile(sf::Vector2i index)
{
    if (openFileQueue.size() >= 400)
    {
        sf::Vector2i idx = openFileQueue.front();
        quadtreeFiles[idx.x][idx.y]->close();

        openFileQueue.pop();
    }

    quadtreeFiles[index.x][index.y]->open(GetQuadtreeFilename(index.x, index.y), std::ios::out | std::ios::binary | std::ios::app);
    openFileQueue.push(index);
}

void Quadtree::AddToIndex(sf::Vector2i quadtreePos, Index index)
{
    std::ofstream* file = quadtreeFiles[quadtreePos.x][quadtreePos.y];
    if (!file->is_open())
    {
        OpenFile(quadtreePos);
    }

    file->write((char*)&index, sizeof(index));
    quadtreeSizes[quadtreePos.x][quadtreePos.y]++;
    if (quadtreeSizes[quadtreePos.x][quadtreePos.y] % 100 == 0)
    {
        quadtreeFiles[quadtreePos.x][quadtreePos.y]->flush();
    }
}

std::string Quadtree::GetQuadtreeFilename(int x, int y)
{
    std::stringstream filename;
    filename << quadtreeFolderName << "/" << x<< "/" << y << ".bin";
    return filename.str();
}

void Quadtree::CreateQuadtreeFiles(std::string quadtreeFolder)
{
    quadtreeFolderName = quadtreeFolder;

    quadtreeFiles.clear();
    quadtreeSizes.clear();
    
    for (int i = 0; i < size; i++)
    {
        std::stringstream folder;
        folder << ".\\" << quadtreeFolder << "\\" << i;
        _mkdir(folder.str().c_str());

        quadtreeFiles.push_back(std::vector<std::ofstream*>());
        quadtreeSizes.push_back(std::vector<long>());
        
        for (int j = 0; j < size; j++)
        {
            quadtreeFiles[i].push_back(new std::ofstream(GetQuadtreeFilename(i, j), std::ios::out | std::ios::binary));
            if (!quadtreeFiles[i][j])
            {
                std::cout << "Error creating the file to write to!" << std::endl;
                return;
            }

            long startingSize = 0;
            quadtreeFiles[i][j]->write((char*)&startingSize, sizeof(long));
            quadtreeFiles[i][j]->close();

            quadtreeSizes[i].push_back(startingSize);
        }

        std::cout << "Created index " << i << " files." << std::endl;
    }
}

void Quadtree::CloseFiles()
{
    // Close all the files in the queue
    while (openFileQueue.size() != 0)
    {
        sf::Vector2i idx = openFileQueue.front();
        quadtreeFiles[idx.x][idx.y]->close();

        openFileQueue.pop();
    }

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            // Update the number of total items in each quadtree array.
            quadtreeFiles[i][j]->open(GetQuadtreeFilename(i, j), std::ios::out | std::ios::binary | std::ios::app);
            quadtreeFiles[i][j]->seekp(0, std::ios::beg);
            quadtreeFiles[i][j]->write((char*)&quadtreeSizes[i][j], sizeof(long));
            quadtreeFiles[i][j]->flush();
            quadtreeFiles[i][j]->close();
        }
    }
}
*/

Quadtree::~Quadtree()
{
}
