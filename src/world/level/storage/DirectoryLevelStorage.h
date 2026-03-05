#pragma once

#ifdef MINECRAFT_LARGE_WORLD
// 51 maps per player (7x7 overworld, 1 nether, 1 end) * 100 players rounded up
// to power of 2
#define MAXIMUM_MAP_SAVE_DATA 8192 // 65536

// 4J Stu - These are special map slots that are used on local machines. They
// will never be an actual map, but are placeholders for when we get updated
// with the correct id
#define MAP_OVERWORLD_DEFAULT_INDEX 65535
#define MAP_NETHER_DEFAULT_INDEX    65534
#define MAP_END_DEFAULT_INDEX       65533
#else
#define MAXIMUM_MAP_SAVE_DATA       256

// 4J Stu - These are special map slots that are used on local machines. They
// will never be an actual map, but are placeholders for when we get updated
// with the correct id
#define MAP_OVERWORLD_DEFAULT_INDEX 255
#define MAP_NETHER_DEFAULT_INDEX    254
#define MAP_END_DEFAULT_INDEX       253
#endif

// The save file version in which we added the End dimension map mappings
#define END_DIMENSION_MAP_MAPPINGS_SAVE_VERSION 5

#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "util/console/ArrayWithLength.h"
#include "util/console/fileio/ConsoleSavePath.h"
#include "util/java/File.h"
#include "win/extraX64.h"

#include "LevelStorage.h"
#include "PlayerIO.h"

class ByteArrayOutputStream;
class ConsoleSaveFile;
class DataInputStream;
class DataOutputStream;

// 4J Stu - Added this which we will write out as a file. Map id's are stored in
// itemInstances as the auxValue, so we can have at most 65536 maps. As we
// currently have a limit of 80 players with 3 maps each we should not hit this
// limit.
typedef struct _MapDataMappings {
    PlayerUID xuids[MAXIMUM_MAP_SAVE_DATA];
    byte      dimensions[MAXIMUM_MAP_SAVE_DATA / 4];

    _MapDataMappings();
    int  getDimension(int id);
    void setMapping(int id, PlayerUID xuid, int dimension);
} MapDataMappings;

// Old version the only used 1 bit for dimension indexing
typedef struct _MapDataMappings_old {
    PlayerUID xuids[MAXIMUM_MAP_SAVE_DATA];
    byte      dimensions[MAXIMUM_MAP_SAVE_DATA / 8];

    _MapDataMappings_old();
    int  getDimension(int id);
    void setMapping(int id, PlayerUID xuid, int dimension);
} MapDataMappings_old;

class DirectoryLevelStorage : public LevelStorage, public PlayerIO {
private:
    /* 4J Jev, Probably no need for this as theres no exceptions being thrown.
    static const Logger *logger = Logger::getLogger("Minecraft"); */

    const File dir;
    // const File playerDir;
    const ConsoleSavePath playerDir;
    // const File dataDir;
    const ConsoleSavePath dataDir;
    const std::int64_t    sessionId;
    const std::wstring    levelId;

    static const std::wstring sc_szPlayerDir;
    // 4J Added
#ifdef MINECRAFT_LARGE_WORLD
    class PlayerMappings {
        friend class DirectoryLevelStorage;

    private:
        std::unordered_map<std::int64_t, short> m_mappings;

    public:
        void
        addMapping(int id, int centreX, int centreZ, int dimension, int scale);
        bool
        getMapping(int& id, int centreX, int centreZ, int dimension, int scale);
        void writeMappings(DataOutputStream* dos);
        void readMappings(DataInputStream* dis);
    };
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)              \
    || defined(_DURANGO)
    std::unordered_map<PlayerUID, PlayerMappings, PlayerUID::Hash>
        m_playerMappings;
#else
    std::unordered_map<PlayerUID, PlayerMappings> m_playerMappings;
#endif
    byteArray m_usedMappings;
#else
    MapDataMappings m_mapDataMappings;
    MapDataMappings m_saveableMapDataMappings;
#endif
    bool m_bHasLoadedMapDataMappings;

    std::unordered_map<std::wstring, ByteArrayOutputStream*> m_cachedSaveData;
    std::vector<short>
        m_mapFilesToDelete; // Temp list of files that couldn't be deleted
                            // immediately due to saving being disabled

protected:
    ConsoleSaveFile* m_saveFile;

public:
    virtual ConsoleSaveFile* getSaveFile() { return m_saveFile; }
    virtual void             flushSaveFile(bool autosave);

public:
    DirectoryLevelStorage(
        ConsoleSaveFile*    saveFile,
        const File          dir,
        const std::wstring& levelId,
        bool                createPlayerDir
    );
    ~DirectoryLevelStorage();

private:
    void initiateSession();

protected:
    File getFolder();

public:
    void                  checkSession();
    virtual ChunkStorage* createChunkStorage(Dimension* dimension);
    LevelData*            prepareLevel();
    virtual void          saveLevelData(
                 LevelData*                            levelData,
                 std::vector<std::shared_ptr<Player>>* players
             );
    virtual void saveLevelData(LevelData* levelData);
    virtual void save(std::shared_ptr<Player> player);
    virtual bool load(
        std::shared_ptr<Player> player
    ); // 4J Changed return val to bool to check if new player or loaded player
    virtual CompoundTag* loadPlayerDataTag(PlayerUID xuid);
    virtual void         clearOldPlayerFiles(); // 4J Added
    PlayerIO*            getPlayerIO();
    virtual void         closeAll();
    ConsoleSavePath      getDataFile(const std::wstring& id);
    std::wstring         getLevelId();

    // 4J Added
    virtual int getAuxValueForMap(
        PlayerUID xuid,
        int       dimension,
        int       centreXC,
        int       centreZC,
        int       scale
    );
    virtual void        saveMapIdLookup();
    virtual void        deleteMapFilesForPlayer(std::shared_ptr<Player> player);
    virtual void        saveAllCachedData();
    void                resetNetherPlayerPositions(); // 4J Added
    static std::wstring getPlayerDir() { return sc_szPlayerDir; }

private:
    void dontSaveMapMappingForPlayer(PlayerUID xuid);
    void deleteMapFilesForPlayer(PlayerUID xuid);
};
