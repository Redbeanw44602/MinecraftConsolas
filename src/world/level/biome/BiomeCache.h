#pragma once

#include <unordered_map>
#include <vector>

#include "util/console/ArrayWithLength.h"
#include "util/java/JavaIntHash.h"

class Biome;
class BiomeSource;

class BiomeCache {
private:
    static const int DECAY_TIME     = 1000 * 30;
    static const int ZONE_SIZE_BITS = 4;
    static const int ZONE_SIZE      = 1 << ZONE_SIZE_BITS;
    static const int ZONE_SIZE_MASK = ZONE_SIZE - 1;

    const BiomeSource* source;
    std::int64_t       lastUpdateTime;

public:
    class Block {
    public:
        // MGH - changed this to just cache biome indices, as we have direct
        // access to the data if we know the index.
        // 		floatArray temps;
        // 		floatArray downfall;
        // 		BiomeArray biomes;
        byteArray    biomeIndices;
        int          x, z;
        std::int64_t lastUse;

        Block(int x, int z, BiomeCache* parent);
        ~Block();
        Biome* getBiome(int x, int z);
        float  getTemperature(int x, int z);
        float  getDownfall(int x, int z);
    };

private:
    std::unordered_map<std::int64_t, Block*, LongKeyHash, LongKeyEq>
                        cached; // 4J - was LongHashMap
    std::vector<Block*> all;    // was ArrayList<Block>

public:
    BiomeCache(BiomeSource* source);
    ~BiomeCache();

    Block*     getBlockAt(int x, int z);
    Biome*     getBiome(int x, int z);
    float      getTemperature(int x, int z);
    float      getDownfall(int x, int z);
    void       update();
    BiomeArray getBiomeBlockAt(int x, int z);
    byteArray  getBiomeIndexBlockAt(int x, int z);

private:
    CRITICAL_SECTION m_CS;
};
