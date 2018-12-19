#pragma once

// Lines are construed to be from the current point to the next point.
struct Index
{
    int stripIdx;
    int pointIdx;

    Index()
    { }

    Index(int si, int pi) : stripIdx(si), pointIdx(pi)
    { }
};