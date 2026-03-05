#pragma once

#include <deque>
#include <vector>

#include <windows.h>

#include <mss.h>

#include "util/console/C4JThread.h"
#include "world/level/biome/Biome.h"
#include "world/level/chunk/ChunkSource.h"

class ChunkStorage;
class LevelChunk;

class ServerLevel;

class ServerChunkCache : public ChunkSource {
private:
    //	std::unordered_set<int,IntKeyHash, IntKeyEq> toDrop;
private:
    LevelChunk*   emptyChunk;
    ChunkSource*  source;
    ChunkStorage* storage;

public:
    bool autoCreate;

private:
    LevelChunk**             cache;
    std::vector<LevelChunk*> m_loadedChunkList;
    ServerLevel*             level;

#ifdef MINECRAFT_LARGE_WORLD
    std::deque<LevelChunk*> m_toDrop;
    LevelChunk**            m_unloadedCache;
#endif

    // 4J - added for multithreaded support
    CRITICAL_SECTION m_csLoadCreate;
    // 4J - size of cache is defined by size of one side - must be even
    int XZSIZE;
    int XZOFFSET;

public:
    ServerChunkCache(
        ServerLevel*  level,
        ChunkStorage* storage,
        ChunkSource*  source
    );
    virtual ~ServerChunkCache();
    virtual bool              hasChunk(int x, int z);
    std::vector<LevelChunk*>* getLoadedChunkList();
    void                      drop(int x, int z);
    void                      dropAll();
    virtual LevelChunk*       create(int x, int z);
    LevelChunk*         create(int x, int z, bool asyncPostProcess); // 4J added
    virtual LevelChunk* getChunk(int x, int z);
#ifdef MINECRAFT_LARGE_WORLD
    LevelChunk* getChunkLoadedOrUnloaded(int x, int z); // 4J added
#endif
    virtual LevelChunk** getCache() { return cache; } // 4J added

    // 4J-JEV Added; Remove chunk from the toDrop queue.
#ifdef MINECRAFT_LARGE_WORLD
    void dontDrop(int x, int z);
#endif

private:
    LevelChunk* load(int x, int z);
    void        saveEntities(LevelChunk* levelChunk);
    void        save(LevelChunk* levelChunk);

    void updatePostProcessFlag(
        short       flag,
        int         x,
        int         z,
        int         xo,
        int         zo,
        LevelChunk* lc
    );                                                      // 4J added
    void updatePostProcessFlags(int x, int z);              // 4J added
    void flagPostProcessComplete(short flag, int x, int z); // 4J added
public:
    virtual void postProcess(ChunkSource* parent, int x, int z);


private:
#ifdef MINECRAFT_LARGE_WORLD
    static const int MAX_SAVES = 20;
#else
    // 4J Stu - Was 24, but lowering it drastically so that we can trickle save
    // chunks
    static const int MAX_SAVES = 1;
#endif

public:
    virtual bool         saveAllEntities();
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

private:
    typedef struct _SaveThreadData {
        ServerChunkCache* cache;
        LevelChunk*       chunkToSave;
        bool              saveEntities;
        bool              useSharedThreadStorage;
        C4JThread::Event* notificationEvent;
        C4JThread::Event* wakeEvent; // This is a handle to the one fired by the
                                     // producer thread
    } SaveThreadData;

public:
    static int runSaveThreadProc(LPVOID lpParam);
};
