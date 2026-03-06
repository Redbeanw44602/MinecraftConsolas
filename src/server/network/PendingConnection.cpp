#include "PendingConnection.h"

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <4J_Storage.h>
#include <mss.h>

#include "network/Connection.h"
#include "network/packet/LoginPacket.h"
#include "network/packet/PreLoginPacket.h"
#include "platform/App_enums.h"
#include "platform/network/NetworkPlayerInterface.h"
#include "platform/network/PlatformNetworkManagerInterface.h"
#include "server/MinecraftServer.h"
#include "server/PlayerList.h"
#include "server/level/ServerPlayer.h"
#include "util/SharedConstants.h"
#include "util/java/Random.h"
#include "win/Windows64_App.h"
#include "win/extraX64.h"

#include "PlayerConnection.h"
#include "ServerConnection.h"

class Packet;
// #ifdef __PS3__
// #include "PS3\Network\NetworkPlayerSony.h"
// #endif

Random* PendingConnection::random = new Random();

PendingConnection::PendingConnection(
    MinecraftServer*    server,
    Socket*             socket,
    const std::wstring& id
) {
    // 4J - added initialisers
    done          = false;
    _tick         = 0;
    name          = L"";
    acceptedLogin = nullptr;
    loginKey      = L"";

    this->server        = server;
    connection          = new Connection(socket, id, this);
    connection->fakeLag = FAKE_LAG;
}

PendingConnection::~PendingConnection() { delete connection; }

void PendingConnection::tick() {
    if (acceptedLogin != NULL) {
        this->handleAcceptedLogin(acceptedLogin);
        acceptedLogin = nullptr;
    }
    if (_tick++ == MAX_TICKS_BEFORE_LOGIN) {
        disconnect(DisconnectPacket::eDisconnect_LoginTooLong);
    } else {
        connection->tick();
    }
}

void PendingConnection::disconnect(DisconnectPacket::eDisconnectReason reason) {
    //   try {	// 4J - removed try/catch
    //        logger.info("Disconnecting " + getName() + ": " + reason);
    app.DebugPrintf("Pending connection disconnect: %d\n", reason);
    connection->send(
        std::shared_ptr<DisconnectPacket>(new DisconnectPacket(reason))
    );
    connection->sendAndQuit();
    done = true;
    //    } catch (Exception e) {
    //        e.printStackTrace();
    //    }
}

void PendingConnection::handlePreLogin(std::shared_ptr<PreLoginPacket> packet) {
    if (packet->m_netcodeVersion != MINECRAFT_NET_VERSION) {
        app.DebugPrintf(
            "Netcode version is %d not equal to %d\n",
            packet->m_netcodeVersion,
            MINECRAFT_NET_VERSION
        );
        if (packet->m_netcodeVersion > MINECRAFT_NET_VERSION) {
            disconnect(DisconnectPacket::eDisconnect_OutdatedServer);
        } else {
            disconnect(DisconnectPacket::eDisconnect_OutdatedClient);
        }
        return;
    }
    //	printf("Server: handlePreLogin\n");
    name = packet->loginKey; // 4J Stu - Change from the login packet as we know
                             // better on client end during the pre-login packet
    sendPreLoginResponse();
}

void PendingConnection::sendPreLoginResponse() {
    // 4J Stu - Calculate the players with UGC privileges set
    PlayerUID* ugcXuids           = new PlayerUID[MINECRAFT_NET_MAX_PLAYERS];
    DWORD      ugcXuidCount       = 0;
    DWORD      hostIndex          = 0;
    BYTE       ugcFriendsOnlyBits = 0;
    char       szUniqueMapName[14];

    StorageManager.GetSaveUniqueFilename(szUniqueMapName);

    PlayerList* playerList = MinecraftServer::getInstance()->getPlayers();
    for (auto it = playerList->players.begin(); it != playerList->players.end();
         ++it) {
        std::shared_ptr<ServerPlayer> player = *it;
        // If the offline Xuid is invalid but the online one is not then that's
        // guest which we should ignore If the online Xuid is invalid but the
        // offline one is not then we are definitely an offline game so dont
        // care about UGC

        // PADDY - this is failing when a local player with chat restrictions
        // joins an online game

        if (player != NULL && player->connection->m_offlineXUID != INVALID_XUID
            && player->connection->m_onlineXUID != INVALID_XUID) {
            if (player->connection->m_friendsOnlyUGC) {
                ugcFriendsOnlyBits |= (1 << ugcXuidCount);
            }
            // Need to use the online XUID otherwise friend checks will fail on
            // the client
            ugcXuids[ugcXuidCount] = player->connection->m_onlineXUID;

            if (player->connection->getNetworkPlayer() != NULL
                && player->connection->getNetworkPlayer()->IsHost())
                hostIndex = ugcXuidCount;

            ++ugcXuidCount;
        }
    }

#if 0
    if (false)//	server->onlineMode) // 4J - removed
	{
        loginKey = L"TOIMPLEMENT"; // 4J - todo Long.toHexString(random.nextLong());
        connection->send( std::shared_ptr<PreLoginPacket>( new PreLoginPacket(loginKey, ugcXuids, ugcXuidCount, ugcFriendsOnlyBits, server->m_ugcPlayersVersion, szUniqueMapName,app.GetGameHostOption(eGameHostOption_All),hostIndex) ) );
    }
	else
#endif
    {
        connection->send(
            std::shared_ptr<PreLoginPacket>(new PreLoginPacket(
                L"-",
                ugcXuids,
                ugcXuidCount,
                ugcFriendsOnlyBits,
                server->m_ugcPlayersVersion,
                szUniqueMapName,
                app.GetGameHostOption(eGameHostOption_All),
                hostIndex,
                server->m_texturePackId
            ))
        );
    }
}

void PendingConnection::handleLogin(std::shared_ptr<LoginPacket> packet) {
    //	printf("Server: handleLogin\n");
    // name = packet->userName;
    if (packet->clientVersion != SharedConstants::NETWORK_PROTOCOL_VERSION) {
        app.DebugPrintf(
            "Client version is %d not equal to %d\n",
            packet->clientVersion,
            SharedConstants::NETWORK_PROTOCOL_VERSION
        );
        if (packet->clientVersion > SharedConstants::NETWORK_PROTOCOL_VERSION) {
            disconnect(DisconnectPacket::eDisconnect_OutdatedServer);
        } else {
            disconnect(DisconnectPacket::eDisconnect_OutdatedClient);
        }
        return;
    }

    // if (true)// 4J removed !server->onlineMode)
    bool sentDisconnect = false;

    if (sentDisconnect) {
        // Do nothing
    } else if (server->getPlayers()->isXuidBanned(packet->m_onlineXuid)) {
        disconnect(DisconnectPacket::eDisconnect_Banned);
    } else {
        handleAcceptedLogin(packet);
    }
    // else
    {
        // 4J - removed
#if 0 
        new Thread() {
            public void run() {
                try {
                    String key = loginKey;
                    URL url = new URL("http://www.minecraft.net/game/checkserver.jsp?user=" + URLEncoder.encode(packet.userName, "UTF-8") + "&serverId=" + URLEncoder.encode(key, "UTF-8"));
                    BufferedReader br = new BufferedReader(new InputStreamReader(url.openStream()));
                    String msg = br.readLine();
                    br.close();
                    if (msg.equals("YES")) {
                        acceptedLogin = packet;
                    } else {
                        disconnect("Failed to verify username!");
                    }
                } catch (Exception e) {
                    disconnect("Failed to verify username! [internal error " + e + "]");
                    e.printStackTrace();
                }
            }
        }.start();
#endif
    }
}

void PendingConnection::handleAcceptedLogin(
    std::shared_ptr<LoginPacket> packet
) {
    if (packet->m_ugcPlayersVersion != server->m_ugcPlayersVersion) {
        // Send the pre-login packet again with the new list of players
        sendPreLoginResponse();
        return;
    }

    // Guests use the online xuid, everyone else uses the offline one
    PlayerUID playerXuid = packet->m_offlineXuid;
    if (playerXuid == INVALID_XUID) playerXuid = packet->m_onlineXuid;

    std::shared_ptr<ServerPlayer> playerEntity =
        server->getPlayers()
            ->getPlayerForLogin(this, name, playerXuid, packet->m_onlineXuid);
    if (playerEntity != NULL) {
        server->getPlayers()->placeNewPlayer(connection, playerEntity, packet);
        connection = NULL; // We've moved responsibility for this over to the
                           // new PlayerConnection, NULL so we don't delete our
                           // reference to it here in our dtor
    }
    done = true;
}

void PendingConnection::onDisconnect(
    DisconnectPacket::eDisconnectReason reason,
    void*                               reasonObjects
) {
    //    logger.info(getName() + " lost connection");
    done = true;
}

void PendingConnection::handleGetInfo(std::shared_ptr<GetInfoPacket> packet) {
    // try {
    // String message = server->motd + "�" + server->players->getPlayerCount() +
    // "�" + server->players->getMaxPlayers(); connection->send(new
    // DisconnectPacket(message));
    connection->send(
        std::shared_ptr<DisconnectPacket>(
            new DisconnectPacket(DisconnectPacket::eDisconnect_ServerFull)
        )
    );
    connection->sendAndQuit();
    server->connection->removeSpamProtection(connection->getSocket());
    done = true;
    //} catch (Exception e) {
    //	e.printStackTrace();
    //}
}

void PendingConnection::handleKeepAlive(
    std::shared_ptr<KeepAlivePacket> packet
) {
    // Ignore
}

void PendingConnection::onUnhandledPacket(std::shared_ptr<Packet> packet) {
    disconnect(DisconnectPacket::eDisconnect_UnexpectedPacket);
}

void PendingConnection::send(std::shared_ptr<Packet> packet) {
    connection->send(packet);
}

std::wstring PendingConnection::getName() {
    return L"Unimplemented";
    //        if (name != null) return name + " [" +
    //        connection.getRemoteAddress().toString() + "]"; return
    //        connection.getRemoteAddress().toString();
}

bool PendingConnection::isServerPacketListener() { return true; }
