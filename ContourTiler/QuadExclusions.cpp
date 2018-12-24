#include <fstream>
#include <iostream>
#include "QuadExclusions.h"

QuadExclusions::QuadExclusions()
    : exclusions()
{
}

void QuadExclusions::SetupExclusions(const std::string& exclusionFilename)
{
    this->exclusionFilename = exclusionFilename;
    std::ifstream exclusionFile(exclusionFilename, std::ios::out | std::ios::binary);
    if (!exclusionFile)
    {
        std::cout << "No exclusion file was found. Unless you want to specifically exclude regions, you can ignore this warning." << std::endl;
        return;
    }

    int size;
    exclusionFile.read((char*)&size, sizeof(int));
    for (int i = 0; i < size; i++)
    {
        sf::Vector2i point;
        exclusionFile.read((char*)&point.x, sizeof(int));
        exclusionFile.read((char*)&point.y, sizeof(int));
        exclusions.insert(point);
    }

    exclusionFile.close();
    std::cout << "Read the exclusion file '" << exclusionFilename << "'." << std::endl;
}

bool QuadExclusions::IsExcluded(const sf::Vector2i& point) const
{
    return exclusions.find(point) != exclusions.end();
}

bool QuadExclusions::ToggleExclusion(const sf::Vector2i& point)
{
    std::set<sf::Vector2i>::iterator pointPos = exclusions.find(point);
    if (exclusions.find(point) != exclusions.end())
    {
        exclusions.erase(pointPos);
        return false;
    }
    else
    {
        exclusions.insert(point);
        return true;
    }
}

void QuadExclusions::WriteExclusions()
{
    std::ofstream exclusionFile(this->exclusionFilename, std::ios::out | std::ios::binary);
    int size = (int)exclusions.size();
    exclusionFile.write((char*)&size, sizeof(int));

    for (std::set<sf::Vector2i>::iterator iter = exclusions.begin(); iter != exclusions.end(); iter++)
    {
        exclusionFile.write((char*)&iter->x, sizeof(int));
        exclusionFile.write((char*)&iter->y, sizeof(int));
    }

    exclusionFile.close();
    std::cout << "Wrote the exclusions file '" << this->exclusionFilename << "' out!" << std::endl;
}