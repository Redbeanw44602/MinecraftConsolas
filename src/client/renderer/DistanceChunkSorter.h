#pragma once

#include <functional>
#include <memory>

class Chunk;
class Entity;

class DistanceChunkSorter {
private:
    double ix, iy, iz;

public:
    DistanceChunkSorter(std::shared_ptr<Entity> player);
    bool operator()(const Chunk* a, const Chunk* b) const;
};
