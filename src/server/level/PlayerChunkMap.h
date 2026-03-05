#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "util/console/ArrayWithLength.h"
#include "util/java/JavaIntHash.h"
#include "world/level/ChunkPos.h"

class Packet;
class ServerLevel;
class ServerPlayer;
class TileEntity;

class PlayerChunkMap {
public:
#ifdef MINECRAFT_LARGE_WORLD
    static const int MAX_VIEW_DISTANCE = 30;
#else
    static const int MAX_VIEW_DISTANCE = 15;
#endif
    static const int MIN_VIEW_DISTANCE               = 3;
    static const int MAX_CHANGES_BEFORE_RESEND       = 10;
    static const int MIN_TICKS_BETWEEN_REGION_UPDATE = 10;

    // 4J - added
    class PlayerChunkAddRequest {
    public:
        int                           x, z;
        std::shared_ptr<ServerPlayer> player;
        PlayerChunkAddRequest(
            int                           x,
            int                           z,
            std::shared_ptr<ServerPlayer> player
        )
        : x(x),
          z(z),
          player(player) {}
    };

    class PlayerChunk {
        friend class PlayerChunkMap;

    private:
        PlayerChunkMap*                            parent; // 4J added
        std::vector<std::shared_ptr<ServerPlayer>> players;
        // int x, z;
        ChunkPos pos;

        shortArray changedTiles;
        int        changes;
        int        xChangeMin, xChangeMax;
        int        yChangeMin, yChangeMax;
        int        zChangeMin, zChangeMax;
        int        ticksToNextRegionUpdate; // 4J added
        bool       prioritised;             // 4J added

    public:
        PlayerChunk(int x, int z, PlayerChunkMap* pcm);
        ~PlayerChunk();

        // 4J Added sendPacket param so we can aggregate the initial send into
        // one much smaller packet
        void add(std::shared_ptr<ServerPlayer> player, bool sendPacket = true);
        void remove(std::shared_ptr<ServerPlayer> player);
        void tileChanged(int x, int y, int z);
        void prioritiseTileChanges(); // 4J added
        void broadcast(std::shared_ptr<Packet> packet);
        bool broadcastChanges(bool allowRegionUpdate); // 4J - added parm

    private:
        void broadcast(std::shared_ptr<TileEntity> te);
    };

public:
    std::vector<std::shared_ptr<ServerPlayer>> players;
    void                                       flagEntitiesToBeRemoved(
                                              unsigned int* flags,
                                              bool*         removedFound
                                          ); // 4J added
private:
    std::unordered_map<std::int64_t, PlayerChunk*, LongKeyHash, LongKeyEq>
                                       chunks; // 4J - was LongHashMap
    std::vector<PlayerChunk*>          changedChunks;
    std::vector<PlayerChunkAddRequest> addRequests;             // 4J added
    void tickAddRequests(std::shared_ptr<ServerPlayer> player); // 4J added

    ServerLevel* level;
    int          radius;
    int          dimension;

public:
    PlayerChunkMap(ServerLevel* level, int dimension, int radius);
    ~PlayerChunkMap();
    ServerLevel* getLevel();
    void         tick();
    bool         hasChunk(int x, int z);

private:
    PlayerChunk* getChunk(int x, int z, bool create);
    void         getChunkAndAddPlayer(
                int                           x,
                int                           z,
                std::shared_ptr<ServerPlayer> player
            ); // 4J added
    void getChunkAndRemovePlayer(
        int                           x,
        int                           z,
        std::shared_ptr<ServerPlayer> player
    ); // 4J added
public:
    void
    broadcastTileUpdate(std::shared_ptr<Packet> packet, int x, int y, int z);
    void tileChanged(int x, int y, int z);
    bool isTrackingTile(int x, int y, int z);        // 4J added
    void prioritiseTileChanges(int x, int y, int z); // 4J added
    void add(std::shared_ptr<ServerPlayer> player);
    void remove(std::shared_ptr<ServerPlayer> player);

private:
    bool chunkInRange(int x, int z, int xc, int zc);

public:
    void move(std::shared_ptr<ServerPlayer> player);
    int  getMaxRange();
    bool
    isPlayerIn(std::shared_ptr<ServerPlayer> player, int xChunk, int zChunk);
    static int convertChunkRangeToBlock(int radius);

    // AP added for Vita
    void setRadius(int newRadius);
};
