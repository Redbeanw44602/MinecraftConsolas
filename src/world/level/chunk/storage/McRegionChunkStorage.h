#pragma once

#include <deque>
#include <unordered_map>

#include <mss.h>

#include "util/console/ArrayWithLength.h"

#include "ChunkStorage.h"

class C4JThread;
class ConsoleSaveFile;
class DataOutputStream;
class Level;
class LevelChunk;

class McRegionChunkStorage : public ChunkStorage {
private:
    const std::wstring      m_prefix;
    ConsoleSaveFile*        m_saveFile;
    static CRITICAL_SECTION cs_memory;

    std::unordered_map<std::int64_t, byteArray> m_entityData;

    static std::deque<DataOutputStream*> s_chunkDataQueue;
    static int                           s_runningThreadCount;
    static C4JThread*                    s_saveThreads[3];

public:
    McRegionChunkStorage(ConsoleSaveFile* saveFile, const std::wstring& prefix);
    ~McRegionChunkStorage();
    static void staticCtor();

    virtual LevelChunk* load(Level* level, int x, int z);
    virtual void        save(Level* level, LevelChunk* levelChunk);
    virtual void        saveEntities(Level* level, LevelChunk* levelChunk);
    virtual void        loadEntities(Level* level, LevelChunk* levelChunk);
    virtual void        tick();
    virtual void        flush();
    virtual void        WaitForAll();                // 4J Added
    virtual void        WaitIfTooManyQueuedChunks(); // 4J Added

private:
    static void WaitForAllSaves();
    static void WaitForSaves();
    static int  runSaveThreadProc(LPVOID lpParam);
};
