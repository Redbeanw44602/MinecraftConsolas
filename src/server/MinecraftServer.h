#pragma once

#include <cstddef>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <mss.h>

#include "util/SharedConstants.h"
#include "util/console/ArrayWithLength.h"
#include "util/console/C4JThread.h"
#include "util/console/fileio/FileHeader.h"
#include "world/level/chunk/ChunkSource.h"

#include "ConsoleInputSource.h"

class CommandDispatcher;
class ConsoleCommands;
class ConsoleInput;
class File;
class INetworkPlayer;
class LevelGenerationOptions;
class LevelStorageSource;
class LevelType;
class PlayerList;
class ProgressRenderer;
class ServerConnection;
class ServerLevel;
class Settings;

#define MINECRAFT_SERVER_SLOW_QUEUE_DELAY 250

typedef struct _LoadSaveDataThreadParam {
    LPVOID             data;
    std::int64_t       fileSize;
    const std::wstring saveName;
    _LoadSaveDataThreadParam(
        LPVOID              data,
        std::int64_t        filesize,
        const std::wstring& saveName
    )
    : data(data),
      fileSize(filesize),
      saveName(saveName) {}
} LoadSaveDataThreadParam;

typedef struct _NetworkGameInitData {
    std::int64_t             seed;
    LoadSaveDataThreadParam* saveData;
    DWORD                    settings;
    LevelGenerationOptions*  levelGen;
    DWORD                    texturePackId;
    bool                     findSeed;
    unsigned int             xzSize;
    unsigned char            hellScale;
    ESavePlatform            savePlatform;

    _NetworkGameInitData() {
        seed          = 0;
        saveData      = NULL;
        settings      = 0;
        levelGen      = NULL;
        texturePackId = 0;
        findSeed      = false;
        xzSize        = LEVEL_LEGACY_WIDTH;
        hellScale     = HELL_LEVEL_LEGACY_SCALE;
        savePlatform  = SAVE_FILE_PLATFORM_LOCAL;
    }
} NetworkGameInitData;


// 4J Stu - 1.0.1 updates the server to implement the ServerInterface class, but
// I don't think we will use any of the functions that defines so not
// implementing here
class MinecraftServer : public ConsoleInputSource {
public:
    static const std::wstring VERSION;
    static const int TICK_STATS_SPAN = SharedConstants::TICKS_PER_SECOND * 5;

    //    static Logger logger = Logger.getLogger("Minecraft");
    static std::unordered_map<std::wstring, int> ironTimers;

private:
    static const int DEFAULT_MINECRAFT_PORT = 25565;
    static const int MS_PER_TICK = 1000 / SharedConstants::TICKS_PER_SECOND;

    // 4J Stu - Added 1.0.1, Not needed
    // std::wstring localIp;
    // int port;
public:
    ServerConnection* connection;
    Settings*         settings;
    ServerLevelArray  levels;

private:
    PlayerList* players;

    // 4J Stu - Added 1.0.1, Not needed
    // long[] tickTimes = new long[TICK_STATS_SPAN];
    // long[][] levelTickTimes;
private:
    ConsoleCommands* commands;
    bool             running;
    bool             m_bLoaded;

public:
    bool stopped;
    int  tickCount;

public:
    std::wstring progressStatus;
    int          progress;

private:
    //	std::vector<Tickable *> tickables = new ArrayList<Tickable>();	// 4J -
    // removed
    CommandDispatcher* commandDispatcher;
    std::vector<ConsoleInput*>
        consoleInput; // 4J - was synchronizedList - TODO - investigate
public:
    bool         onlineMode;
    bool         animals;
    bool         npcs;
    bool         pvp;
    bool         allowFlight;
    std::wstring motd;
    int          maxBuildHeight;

private:
    // 4J Added
    // int m_lastSentDifficulty;

public:
    // 4J Stu - This value should be incremented every time the list of players
    // with friends-only UGC settings changes It is sent with PreLoginPacket and
    // compared when it comes back in the LoginPacket
    DWORD m_ugcPlayersVersion;

    // This value is used to store the texture pack id for the currently loaded
    // world
    DWORD m_texturePackId;

public:
    MinecraftServer();
    ~MinecraftServer();

private:
    // 4J Added - LoadSaveDataThreadParam
    bool initServer(
        std::int64_t         seed,
        NetworkGameInitData* initData,
        DWORD                initSettings,
        bool                 findSeed
    );
    void postProcessTerminate(ProgressRenderer* mcprogress);
    bool loadLevel(
        LevelStorageSource*  storageSource,
        const std::wstring&  name,
        std::int64_t         levelSeed,
        LevelType*           pLevelType,
        NetworkGameInitData* initData
    );
    void setProgress(const std::wstring& status, int progress);
    void endProgress();
    void saveAllChunks();
    void saveGameRules();
    void stopServer();

public:
    void               setMaxBuildHeight(int maxBuildHeight);
    int                getMaxBuildHeight();
    PlayerList*        getPlayers();
    void               setPlayers(PlayerList* players);
    ServerConnection*  getConnection();
    bool               isAnimals();
    void               setAnimals(bool animals);
    bool               isNpcsEnabled();
    void               setNpcsEnabled(bool npcs);
    bool               isPvpAllowed();
    void               setPvpAllowed(bool pvp);
    bool               isFlightAllowed();
    void               setFlightAllowed(bool allowFlight);
    bool               isNetherEnabled();
    bool               isHardcore();
    CommandDispatcher* getCommandDispatcher();

public:
    void halt();
    void run(std::int64_t seed, void* lpParameter);

    void broadcastStartSavingPacket();
    void broadcastStopSavingPacket();

private:
    void tick();

public:
    void
    handleConsoleInput(const std::wstring& msg, ConsoleInputSource* source);
    void handleConsoleInputs();
    //    void addTickable(Tickable tickable);	// 4J removed
    static void main(std::int64_t seed, void* lpParameter);
    static void HaltServer(bool bPrimaryPlayerSignedOut = false);

    File*        getFile(const std::wstring& name);
    void         info(const std::wstring& string);
    void         warn(const std::wstring& string);
    std::wstring getConsoleName();
    ServerLevel* getLevel(int dimension);
    void         setLevel(int dimension, ServerLevel* level); // 4J added
    static MinecraftServer* getInstance() { return server; }  // 4J added
    static bool             serverHalted() { return s_bServerHalted; }
    static bool saveOnExitAnswered() { return s_bSaveOnExitAnswered; }
    static void resetFlags() {
        s_bServerHalted       = false;
        s_bSaveOnExitAnswered = false;
    }

    bool flagEntitiesToBeRemoved(unsigned int* flags); // 4J added
private:
    // 4J Added
    static MinecraftServer* server;

    static bool         setTimeOfDayAtEndOfTick;
    static std::int64_t setTimeOfDay;
    static bool         setTimeAtEndOfTick;
    static std::int64_t setTime;

    static bool
        m_bPrimaryPlayerSignedOut; // 4J-PB added to tell the stopserver not to
                                   // save the game - another player may have
                                   // signed in in their place, so
                                   // ProfileManager.IsSignedIn isn't enough
    static bool s_bServerHalted; // 4J Stu Added so that we can halt the server
                                 // even before it's been created properly
    static bool s_bSaveOnExitAnswered; // 4J Stu Added so that we only ask this
                                       // question once when we exit

    // 4J - added so that we can have a separate thread for post processing
    // chunks on level creation
    static int runPostUpdate(void* lpParam);
    C4JThread* m_postUpdateThread;
    bool       m_postUpdateTerminate;
    class postProcessRequest {
    public:
        int          x, z;
        ChunkSource* chunkSource;
        postProcessRequest(int x, int z, ChunkSource* chunkSource)
        : x(x),
          z(z),
          chunkSource(chunkSource) {}
    };
    std::vector<postProcessRequest> m_postProcessRequests;
    CRITICAL_SECTION                m_postProcessCS;

public:
    void addPostProcessRequest(ChunkSource* chunkSource, int x, int z);

public:
    static PlayerList* getPlayerList() {
        if (server != NULL) return server->players;
        else return NULL;
    }
    static void SetTimeOfDay(std::int64_t time) {
        setTimeOfDayAtEndOfTick = true;
        setTimeOfDay            = time;
    }
    static void SetTime(std::int64_t time) {
        setTimeAtEndOfTick = true;
        setTime            = time;
    }

    C4JThread::Event* m_serverPausedEvent;

private:
    // 4J Added
    bool m_isServerPaused;

    // 4J Added - A static that stores the QNet index of the player that is next
    // allowed to send a packet in the slow queue
    static int s_slowQueuePlayerIndex;
    static int s_slowQueueLastTime;

public:
    static bool s_slowQueuePacketSent;

    bool IsServerPaused() { return m_isServerPaused; }

private:
    // 4J Added
    bool m_saveOnExit;
    bool m_suspending;

public:
    // static int getSlowQueueIndex() { return s_slowQueuePlayerIndex; }
    static bool canSendOnSlowQueue(INetworkPlayer* player);
    static void cycleSlowQueueIndex();

    void setSaveOnExit(bool save) {
        m_saveOnExit          = save;
        s_bSaveOnExitAnswered = true;
    }
    void Suspend();
    bool IsSuspending();

    // 4J Stu - A load of functions were all added in 1.0.1 in the
    // ServerInterface, but I don't think we need any of them
};
