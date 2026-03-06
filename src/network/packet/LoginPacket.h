#pragma once

#include <memory>

#include <mss.h>

#include "win/extraX64.h"

#include "Packet.h"

class LevelType;

class LoginPacket : public Packet,
                    public std::enable_shared_from_this<LoginPacket> {
public:
    int          clientVersion;
    std::wstring userName;
    std::int64_t seed;
    char         dimension;
    PlayerUID    m_offlineXuid, m_onlineXuid;    // 4J Added
    char         difficulty;                     // 4J Added
    bool         m_friendsOnlyUGC;               // 4J Added
    DWORD        m_ugcPlayersVersion;            // 4J Added
    INT          m_multiplayerInstanceId;        // 4J Added for sentient
    BYTE         m_playerIndex;                  // 4J Added
    DWORD        m_playerSkinId, m_playerCapeId; // 4J Added
    bool         m_isGuest;                      // 4J Added
    bool         m_newSeaLevel;                  // 4J Added
    LevelType*   m_pLevelType;
    unsigned int m_uiGamePrivileges;
    int          m_xzSize;    // 4J Added
    int          m_hellScale; // 4J Added

    // 1.8.2
    int  gameType;
    BYTE mapHeight;
    BYTE maxPlayers;

    LoginPacket();
    LoginPacket(
        const std::wstring& userName,
        int                 clientVersion,
        LevelType*          pLevelType,
        std::int64_t        seed,
        int                 gameType,
        char                dimension,
        BYTE                mapHeight,
        BYTE                maxPlayers,
        char                difficulty,
        INT                 m_multiplayerInstanceId,
        BYTE                playerIndex,
        bool                newSeaLevel,
        unsigned int        uiGamePrivileges,
        int                 xzSize,
        int                 hellScale
    ); // Server -> Client
    LoginPacket(
        const std::wstring& userName,
        int                 clientVersion,
        PlayerUID           offlineXuid,
        PlayerUID           onlineXuid,
        bool                friendsOnlyUGC,
        DWORD               ugcPlayersVersion,
        DWORD               skinId,
        DWORD               capeId,
        bool                isGuest
    ); // Client -> Server

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int  getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new LoginPacket());
    }
    virtual int getId() { return 1; }
};
