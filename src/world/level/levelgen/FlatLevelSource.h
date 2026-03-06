#pragma once


#include "util/console/ArrayWithLength.h"
#include "world/level/biome/Biome.h"
#include "world/level/chunk/ChunkSource.h"

class Level;
class Random;
class VillageFeature;

class FlatLevelSource : public ChunkSource {
public:
    static const int CHUNK_HEIGHT = 8;
    static const int CHUNK_WIDTH  = 4;

private:
    Level*  level;
    Random* random;
    Random* pprandom;

    bool            generateStructures;
    VillageFeature* villageFeature; // = new VillageFeature(1);

public:
    FlatLevelSource(Level* level, std::int64_t seed, bool generateStructures);
    ~FlatLevelSource();

private:
    void prepareHeights(byteArray blocks);

public:
    virtual LevelChunk*  create(int x, int z);
    virtual LevelChunk*  getChunk(int xOffs, int zOffs);
    virtual bool         hasChunk(int x, int y);
    virtual void         postProcess(ChunkSource* parent, int xt, int zt);
    virtual bool         save(bool force, ProgressListener* progressListener);
    virtual bool         tick();
    virtual bool         shouldSave();
    virtual std::wstring gatherStats();
    virtual std::vector<Biome::MobSpawnerData*>*
                     getMobsAt(MobCategory* mobCategory, int x, int y, int z);
    virtual TilePos* findNearestMapFeature(
        Level*              level,
        const std::wstring& featureName,
        int                 x,
        int                 y,
        int                 z
    );
};
