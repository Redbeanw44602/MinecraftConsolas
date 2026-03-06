#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "util/console/ArrayWithLength.h"
#include "win/extraX64.h"

class Connection;
class GameType;
class Level;
class LoginPacket;
class MinecraftServer;
class Packet;
class PendingConnection;
class Player;
class PlayerIO;
class ProgressListener;
class ServerLevel;
class ServerPlayer;

class PlayerList {
private:
    static const int SEND_PLAYER_INFO_INTERVAL =
        20 * 10; // 4J - brought forward from 1.2.3
    //    public static Logger logger = Logger.getLogger("Minecraft");
public:
    std::vector<std::shared_ptr<ServerPlayer>> players;

private:
    MinecraftServer* server;
    unsigned int     maxPlayers;

    // 4J Added
    std::vector<PlayerUID> m_bannedXuids;
    std::deque<BYTE>       m_smallIdsToKick;
    CRITICAL_SECTION       m_kickPlayersCS;
    std::deque<BYTE>       m_smallIdsToClose;
    CRITICAL_SECTION       m_closePlayersCS;
    /* 4J - removed
        Set<String> bans = new HashSet<String>();
        Set<String> ipBans = new HashSet<String>();
        Set<String> ops = new HashSet<String>();
        Set<String> whitelist = new HashSet<String>();
        File banFile, ipBanFile, opFile, whiteListFile;
        */
    PlayerIO* playerIo;
    bool      doWhiteList;

    GameType* overrideGameMode;
    bool      allowCheatsForAllPlayers;
    int       viewDistance;

    int sendAllPlayerInfoIn;

    // 4J Added to maintain which players in which dimensions can receive all
    // packet types
    std::vector<std::shared_ptr<ServerPlayer>> receiveAllPlayers[3];

private:
    std::shared_ptr<ServerPlayer>
    findAlivePlayerOnSystem(std::shared_ptr<ServerPlayer> currentPlayer);

public:
    void removePlayerFromReceiving(
        std::shared_ptr<ServerPlayer> player,
        bool                          usePlayerDimension = true,
        int                           dimension          = 0
    );
    void addPlayerToReceiving(std::shared_ptr<ServerPlayer> player);
    bool canReceiveAllPackets(std::shared_ptr<ServerPlayer> player);

public:
    PlayerList(MinecraftServer* server);
    ~PlayerList();
    void placeNewPlayer(
        Connection*                   connection,
        std::shared_ptr<ServerPlayer> player,
        std::shared_ptr<LoginPacket>  packet
    );
    void setLevel(ServerLevelArray levels);
    void
    changeDimension(std::shared_ptr<ServerPlayer> player, ServerLevel* from);
    int  getMaxRange();
    bool load(
        std::shared_ptr<ServerPlayer> player
    ); // 4J Changed return val to bool to check if new player or loaded player
protected:
    void save(std::shared_ptr<ServerPlayer> player);

public:
    void validatePlayerSpawnPosition(
        std::shared_ptr<ServerPlayer> player
    ); // 4J Added
    void                          add(std::shared_ptr<ServerPlayer> player);
    void                          move(std::shared_ptr<ServerPlayer> player);
    void                          remove(std::shared_ptr<ServerPlayer> player);
    std::shared_ptr<ServerPlayer> getPlayerForLogin(
        PendingConnection*  pendingConnection,
        const std::wstring& userName,
        PlayerUID           xuid,
        PlayerUID           OnlineXuid
    );
    std::shared_ptr<ServerPlayer> respawn(
        std::shared_ptr<ServerPlayer> serverPlayer,
        int                           targetDimension,
        bool                          keepAllPlayerData
    );
    void
    toggleDimension(std::shared_ptr<ServerPlayer> player, int targetDimension);
    void tick();
    bool isTrackingTile(int x, int y, int z, int dimension);        // 4J added
    void prioritiseTileChanges(int x, int y, int z, int dimension); // 4J added
    void broadcastAll(std::shared_ptr<Packet> packet);
    void broadcastAll(std::shared_ptr<Packet> packet, int dimension);

    std::wstring getPlayerNames();

public:
    bool isWhiteListed(const std::wstring& name);
    bool isOp(const std::wstring& name);
    bool isOp(std::shared_ptr<ServerPlayer> player); // 4J Added
    std::shared_ptr<ServerPlayer> getPlayer(const std::wstring& name);
    std::shared_ptr<ServerPlayer> getPlayer(PlayerUID uid);
    void sendMessage(const std::wstring& name, const std::wstring& message);
    void broadcast(
        double                  x,
        double                  y,
        double                  z,
        double                  range,
        int                     dimension,
        std::shared_ptr<Packet> packet
    );
    void broadcast(
        std::shared_ptr<Player> except,
        double                  x,
        double                  y,
        double                  z,
        double                  range,
        int                     dimension,
        std::shared_ptr<Packet> packet
    );
    void broadcastToAllOps(const std::wstring& message);
    bool sendTo(const std::wstring& name, std::shared_ptr<Packet> packet);
    // 4J Added ProgressListener *progressListener param and bDeleteGuestMaps
    // param
    void
    saveAll(ProgressListener* progressListener, bool bDeleteGuestMaps = false);
    void whiteList(const std::wstring& playerName);
    void blackList(const std::wstring& playerName);
    //    Set<String> getWhiteList();		/ 4J removed
    void reloadWhitelist();
    void
    sendLevelInfo(std::shared_ptr<ServerPlayer> player, ServerLevel* level);
    void             sendAllPlayerInfo(std::shared_ptr<ServerPlayer> player);
    int              getPlayerCount();
    int              getPlayerCount(ServerLevel* level); // 4J Added
    int              getMaxPlayers();
    MinecraftServer* getServer();
    int              getViewDistance();
    void             setOverrideGameMode(GameType* gameMode);

private:
    void updatePlayerGameMode(
        std::shared_ptr<ServerPlayer> newPlayer,
        std::shared_ptr<ServerPlayer> oldPlayer,
        Level*                        level
    );

public:
    void setAllowCheatsForAllPlayers(bool allowCommands);

    // 4J Added
    void kickPlayerByShortId(BYTE networkSmallId);
    void closePlayerConnectionBySmallId(BYTE networkSmallId);
    bool isXuidBanned(PlayerUID xuid);
    // AP added for Vita so the range can be increased once the level starts
    void setViewDistance(int newViewDistance);
};
