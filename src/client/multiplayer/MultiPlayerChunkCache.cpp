#include "MultiPlayerChunkCache.h"

#include <cstddef>
#include <type_traits>
#include <utility>

#include "platform/network/GameNetworkManager.h"
#include "server/MinecraftServer.h"
#include "server/level/ServerChunkCache.h"
#include "server/level/ServerLevel.h"
#include "util/console/ArrayWithLength.h"
#include "util/console/StringHelpers.h"
#include "world/level/Level.h"
#include "world/level/LevelType.h"
#include "world/level/LightLayer.h"
#include "world/level/chunk/EmptyLevelChunk.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/chunk/WaterLevelChunk.h"
#include "world/level/dimension/Dimension.h"
#include "world/level/storage/LevelData.h"
#include "world/level/tile/Tile.h"

MultiPlayerChunkCache::MultiPlayerChunkCache(Level* level) {
    XZSIZE   = level->dimension->getXZSize(); // 4J Added
    XZOFFSET = XZSIZE / 2;                    // 4J Added
    m_XZSize = XZSIZE;
    hasData  = new bool[XZSIZE * XZSIZE];
    memset(hasData, 0, sizeof(bool) * XZSIZE * XZSIZE);

    emptyChunk = new EmptyLevelChunk(
        level,
        byteArray(16 * 16 * Level::maxBuildHeight),
        0,
        0
    );

    // For normal world dimension, create a chunk that can be used to create the
    // illusion of infinite water at the edge of the world
    if (level->dimension->id == 0) {
        byteArray bytes = byteArray(16 * 16 * 128);

        // Superflat.... make grass, not water...
        if (level->getLevelData()->getGenerator() == LevelType::lvl_flat) {
            for (int x = 0; x < 16; x++)
                for (int y = 0; y < 128; y++)
                    for (int z = 0; z < 16; z++) {
                        unsigned char tileId = 0;
                        if (y == 3) tileId = Tile::grass_Id;
                        else if (y <= 2) tileId = Tile::dirt_Id;

                        bytes[x << 11 | z << 7 | y] = tileId;
                    }
        } else {
            for (int x = 0; x < 16; x++)
                for (int y = 0; y < 128; y++)
                    for (int z = 0; z < 16; z++) {
                        unsigned char tileId = 0;
                        if (y <= (level->getSeaLevel() - 10))
                            tileId = Tile::rock_Id;
                        else if (y < level->getSeaLevel())
                            tileId = Tile::calmWater_Id;

                        bytes[x << 11 | z << 7 | y] = tileId;
                    }
        }

        waterChunk = new WaterLevelChunk(level, bytes, 0, 0);

        delete[] bytes.data;

        if (level->getLevelData()->getGenerator() == LevelType::lvl_flat) {
            for (int x = 0; x < 16; x++)
                for (int y = 0; y < 128; y++)
                    for (int z = 0; z < 16; z++) {
                        if (y >= 3) {
                            ((WaterLevelChunk*)waterChunk)
                                ->setLevelChunkBrightness(
                                    LightLayer::Sky,
                                    x,
                                    y,
                                    z,
                                    15
                                );
                        }
                    }
        } else {
            for (int x = 0; x < 16; x++)
                for (int y = 0; y < 128; y++)
                    for (int z = 0; z < 16; z++) {
                        if (y >= (level->getSeaLevel() - 1)) {
                            ((WaterLevelChunk*)waterChunk)
                                ->setLevelChunkBrightness(
                                    LightLayer::Sky,
                                    x,
                                    y,
                                    z,
                                    15
                                );
                        } else {
                            ((WaterLevelChunk*)waterChunk)
                                ->setLevelChunkBrightness(
                                    LightLayer::Sky,
                                    x,
                                    y,
                                    z,
                                    2
                                );
                        }
                    }
        }
    } else {
        waterChunk = NULL;
    }

    this->level = level;

    this->cache = new LevelChunk*[XZSIZE * XZSIZE];
    memset(this->cache, 0, XZSIZE * XZSIZE * sizeof(LevelChunk*));
    InitializeCriticalSectionAndSpinCount(&m_csLoadCreate, 4000);
}

MultiPlayerChunkCache::~MultiPlayerChunkCache() {
    delete emptyChunk;
    delete waterChunk;
    delete cache;
    delete hasData;

    auto itEnd = loadedChunkList.end();
    for (auto it = loadedChunkList.begin(); it != itEnd; it++) delete *it;

    DeleteCriticalSection(&m_csLoadCreate);
}


bool MultiPlayerChunkCache::hasChunk(int x, int z) {
    // This cache always claims to have chunks, although it might actually just
    // return empty data if it doesn't have anything
    return true;
}

// 4J  added - find out if we actually really do have a chunk in our cache
bool MultiPlayerChunkCache::reallyHasChunk(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    // Check we're in range of the stored level - if we aren't, then consider
    // that we do have that chunk as we'll be able to use the water chunk there
    if ((ix < 0) || (ix >= XZSIZE)) return true;
    if ((iz < 0) || (iz >= XZSIZE)) return true;
    int idx = ix * XZSIZE + iz;

    LevelChunk* chunk = cache[idx];
    if (chunk == NULL) {
        return false;
    }
    return hasData[idx];
}

void MultiPlayerChunkCache::drop(int x, int z) {
    // 4J Stu - We do want to drop any entities in the chunks, especially for
    // the case when a player is dead as they will not get the RemoveEntity
    // packet if an entity is removed.
    LevelChunk* chunk = getChunk(x, z);
    if (!chunk->isEmpty()) {
        // Added parameter here specifies that we don't want to delete tile
        // entities, as they won't get recreated unless they've got update
        // packets The tile entities are in general only created on the client
        // by virtue of the chunk rebuild
        chunk->unload(false);

        // 4J - We just want to clear out the entities in the chunk, but
        // everything else should be valid
        chunk->loaded = true;
    }
}

LevelChunk* MultiPlayerChunkCache::create(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    // Check we're in range of the stored level
    if ((ix < 0) || (ix >= XZSIZE))
        return (waterChunk ? waterChunk : emptyChunk);
    if ((iz < 0) || (iz >= XZSIZE))
        return (waterChunk ? waterChunk : emptyChunk);
    int         idx       = ix * XZSIZE + iz;
    LevelChunk* chunk     = cache[idx];
    LevelChunk* lastChunk = chunk;

    if (chunk == NULL) {
        EnterCriticalSection(&m_csLoadCreate);

        // LevelChunk *chunk;
        if (g_NetworkManager.IsHost()) // force here to disable sharing of data
        {
            // 4J-JEV: We are about to use shared data, abort if the server is
            // stopped and the data is deleted.
            if (MinecraftServer::getInstance()->serverHalted()) return NULL;

            // If we're the host, then don't create the chunk, share data from
            // the server's copy
#ifdef MINECRAFT_LARGE_WORLD
            LevelChunk* serverChunk =
                MinecraftServer::getInstance()
                    ->getLevel(level->dimension->id)
                    ->cache->getChunkLoadedOrUnloaded(x, z);
#else
            LevelChunk* serverChunk = MinecraftServer::getInstance()
                                          ->getLevel(level->dimension->id)
                                          ->cache->getChunk(x, z);
#endif
            chunk = new LevelChunk(level, x, z, serverChunk);
            // Let renderer know that this chunk has been created - it might
            // have made render data from the EmptyChunk if it got to a chunk
            // before the server sent it
            level->setTilesDirty(
                x * 16,
                0,
                z * 16,
                x * 16 + 15,
                127,
                z * 16 + 15
            );
            hasData[idx] = true;
        } else {
            // Passing an empty array into the LevelChunk ctor, which it now
            // detects and sets up the chunk as compressed & empty
            byteArray bytes;

            chunk = new LevelChunk(level, bytes, x, z);

            // 4J - changed to use new methods for lighting
            chunk->setSkyLightDataAllBright();
            //			Arrays::fill(chunk->skyLight->data, (byte) 255);
        }

        chunk->loaded = true;

        LeaveCriticalSection(&m_csLoadCreate);

#if (defined _WIN64 || defined __LP64__)
        if (InterlockedCompareExchangeRelease64(
                (LONG64*)&cache[idx],
                (LONG64)chunk,
                (LONG64)lastChunk
            )
            == (LONG64)lastChunk)
#else
        if (InterlockedCompareExchangeRelease(
                (LONG*)&cache[idx],
                (LONG)chunk,
                (LONG)lastChunk
            )
            == (LONG)lastChunk)
#endif // _DURANGO
        {
            // If we're sharing with the server, we'll need to calculate our
            // heightmap now, which isn't shared. If we aren't sharing with the
            // server, then this will be calculated when the chunk data arrives.
            if (g_NetworkManager.IsHost()) {
                chunk->recalcHeightmapOnly();
            }

            // Successfully updated the cache
            EnterCriticalSection(&m_csLoadCreate);
            loadedChunkList.push_back(chunk);
            LeaveCriticalSection(&m_csLoadCreate);
        } else {
            // Something else must have updated the cache. Return that chunk and
            // discard this one. This really shouldn't be happening in
            // multiplayer
            delete chunk;
            return cache[idx];
        }

    } else {
        chunk->load();
    }

    return chunk;
}

LevelChunk* MultiPlayerChunkCache::getChunk(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    // Check we're in range of the stored level
    if ((ix < 0) || (ix >= XZSIZE))
        return (waterChunk ? waterChunk : emptyChunk);
    if ((iz < 0) || (iz >= XZSIZE))
        return (waterChunk ? waterChunk : emptyChunk);
    int idx = ix * XZSIZE + iz;

    LevelChunk* chunk = cache[idx];
    if (chunk == NULL) {
        return emptyChunk;
    } else {
        return chunk;
    }
}

bool MultiPlayerChunkCache::save(
    bool              force,
    ProgressListener* progressListener
) {
    return true;
}

bool MultiPlayerChunkCache::tick() { return false; }

bool MultiPlayerChunkCache::shouldSave() { return false; }

void MultiPlayerChunkCache::postProcess(ChunkSource* parent, int x, int z) {}

std::vector<Biome::MobSpawnerData*>* MultiPlayerChunkCache::getMobsAt(
    MobCategory* mobCategory,
    int          x,
    int          y,
    int          z
) {
    return NULL;
}

TilePos* MultiPlayerChunkCache::findNearestMapFeature(
    Level*              level,
    const std::wstring& featureName,
    int                 x,
    int                 y,
    int                 z
) {
    return NULL;
}

std::wstring MultiPlayerChunkCache::gatherStats() {
    EnterCriticalSection(&m_csLoadCreate);
    int size = (int)loadedChunkList.size();
    LeaveCriticalSection(&m_csLoadCreate);
    return L"MultiplayerChunkCache: " + _toString<int>(size);
}

void MultiPlayerChunkCache::dataReceived(int x, int z) {
    int ix = x + XZOFFSET;
    int iz = z + XZOFFSET;
    // Check we're in range of the stored level
    if ((ix < 0) || (ix >= XZSIZE)) return;
    if ((iz < 0) || (iz >= XZSIZE)) return;
    int idx      = ix * XZSIZE + iz;
    hasData[idx] = true;
}
