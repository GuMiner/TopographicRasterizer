#pragma once
#include <set>
#include <SFML\System.hpp>

struct Vec2iComparator
{
    bool operator()(const sf::Vector2i& left, const sf::Vector2i& right) const
    {
        double superSize = 100000;
        return left.x + left.y * superSize < right.x + right.y * superSize;
    }
};

class QuadExclusions
{
    std::set<sf::Vector2i, Vec2iComparator> exclusions;

public:
    QuadExclusions();
    void ReadExclusions();
    void WriteExclusions();

    bool IsExcluded(const sf::Vector2i& point) const;
    bool ToggleExclusion(const sf::Vector2i& point);
};

