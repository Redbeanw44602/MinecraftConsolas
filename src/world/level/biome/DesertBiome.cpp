#include "DesertBiome.h"

#include <vector>

#include "util/java/Random.h"
#include "world/level/Level.h"
#include "world/level/levelgen/feature/DesertWellFeature.h"
#include "world/level/levelgen/feature/Feature.h"
#include "world/level/tile/Tile.h"

#include "Biome.h"
#include "BiomeDecorator.h"

DesertBiome::DesertBiome(int id) : Biome(id) {
    // remove default mob spawn settings
    friendlies.clear();
    friendlies_chicken.clear(); // 4J added
    friendlies_wolf.clear();    // 4J added
    this->topMaterial = (BYTE)Tile::sand_Id;
    this->material    = (BYTE)Tile::sand_Id;

    decorator->treeCount     = -999;
    decorator->deadBushCount = 2;
    decorator->reedsCount    = 50;
    decorator->cactusCount   = 10;
}

void DesertBiome::decorate(Level* level, Random* random, int xo, int zo) {
    Biome::decorate(level, random, xo, zo);

    if (random->nextInt(1000) == 0) {
        int      x    = xo + random->nextInt(16) + 8;
        int      z    = zo + random->nextInt(16) + 8;
        Feature* well = new DesertWellFeature();
        well->place(level, random, x, level->getHeightmap(x, z) + 1, z);
    }
}
