#pragma once

#include <functional>
#include <memory>

class Chunk;
class Mob;

class DirtyChunkSorter {
private:
    std::shared_ptr<Mob> cameraEntity;
    int                  playerIndex; // 4J added

public:
    DirtyChunkSorter(
        std::shared_ptr<Mob> cameraEntity,
        int                  playerIndex
    ); // 4J - added player index
    bool operator()(const Chunk* a, const Chunk* b) const;
};
