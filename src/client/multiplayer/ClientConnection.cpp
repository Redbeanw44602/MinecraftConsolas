#include "ClientConnection.h"

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <4J_Profile.h>

#include "client/Minecraft.h"
#include "client/ProgressRenderer.h"
#include "client/User.h"
#include "client/gui/Gui.h"
#include "client/particle/CritParticle.h"
#include "client/particle/ParticleEngine.h"
#include "client/particle/TakeAnimationParticle.h"
#include "client/player/LocalPlayer.h"
#include "client/player/RemotePlayer.h"
#include "client/renderer/LevelRenderer.h"
#include "client/skins/TexturePackRepository.h"
#include "network/packet/AddEntityPacket.h"
#include "network/packet/AddExperienceOrbPacket.h"
#include "network/packet/AddGlobalEntityPacket.h"
#include "network/packet/AddMobPacket.h"
#include "network/packet/AddPaintingPacket.h"
#include "network/packet/AddPlayerPacket.h"
#include "network/packet/AnimatePacket.h"
#include "network/packet/AwardStatPacket.h"
#include "network/packet/BlockRegionUpdatePacket.h"
#include "network/packet/ChatPacket.h"
#include "network/packet/ChunkTilesUpdatePacket.h"
#include "network/packet/ChunkVisibilityAreaPacket.h"
#include "network/packet/ChunkVisibilityPacket.h"
#include "network/packet/ComplexItemDataPacket.h"
#include "network/packet/ContainerAckPacket.h"
#include "network/packet/ContainerClosePacket.h"
#include "network/packet/ContainerOpenPacket.h"
#include "network/packet/ContainerSetContentPacket.h"
#include "network/packet/ContainerSetDataPacket.h"
#include "network/packet/ContainerSetSlotPacket.h"
#include "network/packet/CustomPayloadPacket.h"
#include "network/packet/EntityActionAtPositionPacket.h"
#include "network/packet/EntityEventPacket.h"
#include "network/packet/ExplodePacket.h"
#include "network/packet/GameEventPacket.h"
#include "network/packet/KeepAlivePacket.h"
#include "network/packet/LevelEventPacket.h"
#include "network/packet/LevelSoundPacket.h"
#include "network/packet/LoginPacket.h"
#include "network/packet/MoveEntityPacket.h"
#include "network/packet/MoveEntityPacketSmall.h"
#include "network/packet/MovePlayerPacket.h"
#include "network/packet/PlayerAbilitiesPacket.h"
#include "network/packet/PlayerInfoPacket.h"
#include "network/packet/PreLoginPacket.h"
#include "network/packet/RemoveEntitiesPacket.h"
#include "network/packet/RemoveMobEffectPacket.h"
#include "network/packet/RespawnPacket.h"
#include "network/packet/RotateHeadPacket.h"
#include "network/packet/ServerSettingsChangedPacket.h"
#include "network/packet/SetEntityDataPacket.h"
#include "network/packet/SetEntityMotionPacket.h"
#include "network/packet/SetEquippedItemPacket.h"
#include "network/packet/SetExperiencePacket.h"
#include "network/packet/SetHealthPacket.h"
#include "network/packet/SetRidingPacket.h"
#include "network/packet/SetSpawnPositionPacket.h"
#include "network/packet/SetTimePacket.h"
#include "network/packet/SignUpdatePacket.h"
#include "network/packet/TakeItemEntityPacket.h"
#include "network/packet/TeleportEntityPacket.h"
#include "network/packet/TextureAndGeometryChangePacket.h"
#include "network/packet/TextureAndGeometryPacket.h"
#include "network/packet/TextureChangePacket.h"
#include "network/packet/TexturePacket.h"
#include "network/packet/TileDestructionPacket.h"
#include "network/packet/TileEntityDataPacket.h"
#include "network/packet/TileEventPacket.h"
#include "network/packet/TileUpdatePacket.h"
#include "network/packet/UpdateGameRuleProgressPacket.h"
#include "network/packet/UpdateMobEffectPacket.h"
#include "network/packet/UpdateProgressPacket.h"
#include "network/packet/XZPacket.h"
#include "platform/App_enums.h"
#include "platform/App_structs.h"
#include "platform/ConsoleGameMode.h"
#include "platform/dlc/DLCManager.h"
#include "platform/dlc/DLCPack.h"
#include "platform/dlc/DLCSkinFile.h"
#include "platform/gamerules/levelrules/ruledefinitions/GameRuleDefinition.h"
#include "platform/network/GameNetworkManager.h"
#include "platform/network/NetworkPlayerInterface.h"
#include "platform/telemetry/TelemetryManager.h"
#include "platform/trial/TrialMode.h"
#include "platform/tutorial/FullTutorialMode.h"
#include "platform/tutorial/Tutorial.h"
#include "platform/tutorial/TutorialEnum.h"
#include "platform/tutorial/TutorialMode.h"
#include "platform/ui/all_platforms/UIEnums.h"
#include "platform/ui/all_platforms/UIStructs.h"
#include "platform/ui/scenes/ingame_menu_screens/containers/UIScene_TradingMenu.h"
#include "server/MinecraftServer.h"
#include "stats/GenericStats.h"
#include "util/MemSect.h"
#include "util/Mth.h"
#include "util/SharedConstants.h"
#include "util/console/StringHelpers.h"
#include "util/java/Class.h"
#include "util/java/InputOutputStream/ByteArrayInputStream.h"
#include "util/java/InputOutputStream/DataInputStream.h"
#include "util/java/Random.h"
#include "util/java/Socket.h"
#include "win/Windows64_App.h"
#include "win/Windows64_UIController.h"
#include "win/extraX64.h"
#include "win/sentient/SentientTelemetryCommon.h"
#include "win/sentient/TelemetryEnum.h"
#include "win/strings.h"
#include "world/ParticleTypes.h"
#include "world/Pos.h"
#include "world/SimpleContainer.h"
#include "world/SoundTypes.h"
#include "world/effect/MobEffectInstance.h"
#include "world/entity/EntityIO.h"
#include "world/entity/ExperienceOrb.h"
#include "world/entity/ItemFrame.h"
#include "world/entity/Mob.h"
#include "world/entity/Painting.h"
#include "world/entity/SynchedEntityData.h"
#include "world/entity/boss/enderdragon/EnderCrystal.h"
#include "world/entity/global/LightningBolt.h"
#include "world/entity/item/Boat.h"
#include "world/entity/item/FallingTile.h"
#include "world/entity/item/ItemEntity.h"
#include "world/entity/item/Minecart.h"
#include "world/entity/item/PrimedTnt.h"
#include "world/entity/monster/Slime.h"
#include "world/entity/npc/ClientSideMerchant.h"
#include "world/entity/player/Abilities.h"
#include "world/entity/player/Inventory.h"
#include "world/entity/player/Player.h"
#include "world/entity/projectile/Arrow.h"
#include "world/entity/projectile/DragonFireball.h"
#include "world/entity/projectile/EyeOfEnderSignal.h"
#include "world/entity/projectile/Fireball.h"
#include "world/entity/projectile/FishingHook.h"
#include "world/entity/projectile/SmallFireball.h"
#include "world/entity/projectile/Snowball.h"
#include "world/entity/projectile/ThrownEgg.h"
#include "world/entity/projectile/ThrownEnderpearl.h"
#include "world/entity/projectile/ThrownExpBottle.h"
#include "world/entity/projectile/ThrownPotion.h"
#include "world/food/FoodConstants.h"
#include "world/food/FoodData.h"
#include "world/inventory/AbstractContainerMenu.h"
#include "world/inventory/Slot.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/MapItem.h"
#include "world/item/trading/Merchant.h"
#include "world/item/trading/MerchantRecipeList.h"
#include "world/level/Explosion.h"
#include "world/level/Level.h"
#include "world/level/LevelSettings.h"
#include "world/level/chunk/LevelChunk.h"
#include "world/level/dimension/Dimension.h"
#include "world/level/saveddata/MapItemSavedData.h"
#include "world/level/storage/LevelData.h"
#include "world/level/storage/SavedDataStorage.h"
#include "world/level/tile/LevelEvent.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/BrewingStandTileEntity.h"
#include "world/level/tile/entity/DispenserTileEntity.h"
#include "world/level/tile/entity/FurnaceTileEntity.h"
#include "world/level/tile/entity/MobSpawnerTileEntity.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "world/level/tile/entity/SkullTileEntity.h"
#include "world/level/tile/entity/TileEntity.h"
#include "world/phys/AABB.h"

#include "MultiPlayerGameMode.h"
#include "MultiPlayerLevel.h"
#include "MultiPlayerLocalPlayer.h"

#ifdef _XBOX
#include "xbox/source files/xui/containers/XUI_Scene_Trading.h"
#else
#endif
#ifdef __PS3__
#include "ps3/source files/network/SonyVoiceChat.h"
#endif
#include "client/skins/DLCTexturePack.h"

class Packet;
class TexturePack;
class UIScene;

#ifdef _DURANGO
#include "stats/DurangoStats.h"
#include "stats/GenericStats.h"
#endif

ClientConnection::ClientConnection(
    Minecraft*          minecraft,
    const std::wstring& ip,
    int                 port
) {
    // 4J Stu - No longer used as we use the socket version below.
    assert(FALSE);
#if 0
	// 4J - added initiliasers
	random = new Random();
	done = false;
    level = false;
    started = false;

    this->minecraft = minecraft;

	Socket *socket;
	if( gNetworkManager.IsHost() )
	{
	    socket = new Socket();	// 4J - Local connection
	}
	else
	{
		socket = new Socket(ip);	// 4J - Connection over xrnm - hardcoded IP at present
	}
	createdOk = socket->createdOk;
	if( createdOk )
	{
	    connection = new Connection(socket, L"Client", this);
	}
	else
	{
		connection = NULL;
		delete socket;
	}
#endif
}

ClientConnection::ClientConnection(
    Minecraft* minecraft,
    Socket*    socket,
    int        iUserIndex /*= -1*/
) {
    // 4J - added initiliasers
    random           = new Random();
    done             = false;
    level            = NULL;
    started          = false;
    savedDataStorage = new SavedDataStorage(NULL);
    maxPlayers       = 20;

    this->minecraft = minecraft;

    if (iUserIndex < 0) {
        m_userIndex = ProfileManager.GetPrimaryPad();
    } else {
        m_userIndex = iUserIndex;
    }

    if (socket == NULL) {
        socket = new Socket(); // 4J - Local connection
    }

    createdOk = socket->createdOk;
    if (createdOk) {
        connection = new Connection(socket, L"Client", this);
    } else {
        connection = NULL;
        // TODO 4J Stu - This will cause issues since the session player owns
        // the socket
        // delete socket;
    }
}

ClientConnection::~ClientConnection() {
    delete connection;
    delete random;
    delete savedDataStorage;
}

void ClientConnection::tick() {
    if (!done) connection->tick();
    connection->flush();
}

INetworkPlayer* ClientConnection::getNetworkPlayer() {
    if (connection != NULL && connection->getSocket() != NULL)
        return connection->getSocket()->getPlayer();
    else return NULL;
}

void ClientConnection::handleLogin(std::shared_ptr<LoginPacket> packet) {
    if (done) return;

    PlayerUID OnlineXuid;
    ProfileManager.GetXUID(m_userIndex, &OnlineXuid, true); // online xuid
    MOJANG_DATA* pMojangData = NULL;

    if (!g_NetworkManager.IsLocalGame()) {
        pMojangData = app.GetMojangDataForXuid(OnlineXuid);
    }

    if (!g_NetworkManager.IsHost()) {
        Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
            (eCCLoginReceived * 100) / (eCCConnected)
        );
    }

    // 4J-PB - load the local player skin (from the global title user storage
    // area) if there is one the primary player on the host machine won't have a
    // qnet player from the socket
    INetworkPlayer* networkPlayer = connection->getSocket()->getPlayer();
    int             iUserID       = -1;

    if (m_userIndex == ProfileManager.GetPrimaryPad()) {
        iUserID = m_userIndex;

        TelemetryManager->SetMultiplayerInstanceId(
            packet->m_multiplayerInstanceId
        );
    } else {
        if (!networkPlayer->IsGuest() && networkPlayer->IsLocal()) {
            // find the pad number of this local player
            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                INetworkPlayer* networkLocalPlayer =
                    g_NetworkManager.GetLocalPlayerByUserIndex(i);
                if (networkLocalPlayer == networkPlayer) {
                    iUserID = i;
                }
            }
        }
    }

    if (iUserID != -1) {
        BYTE* pBuffer = NULL;
        DWORD dwSize  = 0;
        bool  bRes;

        // if there's a special skin or cloak for this player, add it in
        if (pMojangData) {
            // a skin?
            if (pMojangData->wchSkin[0] != 0L) {
                std::wstring wstr = pMojangData->wchSkin;
                // check the file is not already in
                bRes = app.IsFileInMemoryTextures(wstr);
                if (!bRes) {
#ifdef _XBOX
                    C4JStorage::ETMSStatus eTMSStatus;
                    eTMSStatus = StorageManager.ReadTMSFile(
                        iUserID,
                        C4JStorage::eGlobalStorage_Title,
                        C4JStorage::eTMS_FileType_Graphic,
                        pMojangData->wchSkin,
                        &pBuffer,
                        &dwSize
                    );

                    bRes = (eTMSStatus == C4JStorage::ETMSStatus_Idle);
#endif
                }

                if (bRes) {
                    app.AddMemoryTextureFile(wstr, pBuffer, dwSize);
                }
            }

            // a cloak?
            if (pMojangData->wchCape[0] != 0L) {
                std::wstring wstr = pMojangData->wchCape;
                // check the file is not already in
                bRes = app.IsFileInMemoryTextures(wstr);
                if (!bRes) {
#ifdef _XBOX
                    C4JStorage::ETMSStatus eTMSStatus;
                    eTMSStatus = StorageManager.ReadTMSFile(
                        iUserID,
                        C4JStorage::eGlobalStorage_Title,
                        C4JStorage::eTMS_FileType_Graphic,
                        pMojangData->wchCape,
                        &pBuffer,
                        &dwSize
                    );
                    bRes = (eTMSStatus == C4JStorage::ETMSStatus_Idle);
#endif
                }

                if (bRes) {
                    app.AddMemoryTextureFile(wstr, pBuffer, dwSize);
                }
            }
        }

        // If we're online, read the banned game list
        app.ReadBannedList(iUserID);
        // mark the level as not checked against banned levels - it'll be
        // checked once the level starts
        app.SetBanListCheck(iUserID, false);
    }

    if (m_userIndex == ProfileManager.GetPrimaryPad()) {
        if (app.GetTutorialMode()) {
            minecraft->gameMode = new FullTutorialMode(
                ProfileManager.GetPrimaryPad(),
                minecraft,
                this
            );
        }
        // check if we're in the trial version
        else if (ProfileManager.IsFullVersion() == false) {
            minecraft->gameMode =
                new TrialMode(ProfileManager.GetPrimaryPad(), minecraft, this);
        } else {
            MemSect(13);
            minecraft->gameMode = new ConsoleGameMode(
                ProfileManager.GetPrimaryPad(),
                minecraft,
                this
            );
            MemSect(0);
        }


        Level* dimensionLevel = minecraft->getLevel(packet->dimension);
        if (dimensionLevel == NULL) {
            level = new MultiPlayerLevel(
                this,
                new LevelSettings(
                    packet->seed,
                    GameType::byId(packet->gameType),
                    false,
                    false,
                    packet->m_newSeaLevel,
                    packet->m_pLevelType,
                    packet->m_xzSize,
                    packet->m_hellScale
                ),
                packet->dimension,
                packet->difficulty
            );

            // 4J Stu - We want to share the SavedDataStorage between levels
            int    otherDimensionId = packet->dimension == 0 ? -1 : 0;
            Level* activeLevel      = minecraft->getLevel(otherDimensionId);
            if (activeLevel != NULL) {
                // Don't need to delete it here as it belongs to a client
                // connection while will delete it when it's done
                // if( level->savedDataStorage != NULL ) delete
                // level->savedDataStorage;
                level->savedDataStorage = activeLevel->savedDataStorage;
            }

            app.DebugPrintf(
                "ClientConnection - DIFFICULTY --- %d\n",
                packet->difficulty
            );
            level->difficulty   = packet->difficulty; // 4J Added
            level->isClientSide = true;
            minecraft->setLevel(level);
        }

        minecraft->player->setPlayerIndex(packet->m_playerIndex);
        minecraft->player->setCustomSkin(app.GetPlayerSkinId(m_userIndex));
        minecraft->player->setCustomCape(app.GetPlayerCapeId(m_userIndex));


        minecraft->createPrimaryLocalPlayer(ProfileManager.GetPrimaryPad());

        minecraft->player->dimension = packet->dimension;
        // minecraft->setScreen(new ReceivingLevelScreen(this));
        minecraft->player->entityId = packet->clientVersion;

        BYTE networkSmallId = getSocket()->getSmallId();
        app.UpdatePlayerInfo(
            networkSmallId,
            packet->m_playerIndex,
            packet->m_uiGamePrivileges
        );
        minecraft->player->setPlayerGamePrivilege(
            Player::ePlayerGamePrivilege_All,
            packet->m_uiGamePrivileges
        );

        // Assume all privileges are on, so that the first message we see only
        // indicates things that have been turned off
        unsigned int startingPrivileges = 0;
        Player::enableAllPlayerPrivileges(startingPrivileges, true);

        if (networkPlayer->IsHost()) {
            Player::setPlayerGamePrivilege(
                startingPrivileges,
                Player::ePlayerGamePrivilege_HOST,
                1
            );
        }

        displayPrivilegeChanges(minecraft->player, startingPrivileges);

        // update the debugoptions
        app.SetGameSettingsDebugMask(
            ProfileManager.GetPrimaryPad(),
            app.GetGameSettingsDebugMask(-1, true)
        );
    } else {
        // 4J-PB - this isn't the level we want
        // level = (MultiPlayerLevel *)minecraft->level;
        level = (MultiPlayerLevel*)minecraft->getLevel(packet->dimension);
        std::shared_ptr<Player> player;

        if (level == NULL) {
            int otherDimensionId = packet->dimension == 0 ? -1 : 0;
            MultiPlayerLevel* activeLevel =
                minecraft->getLevel(otherDimensionId);

            if (activeLevel == NULL) {
                otherDimensionId = packet->dimension == 0
                                     ? 1
                                     : (packet->dimension == -1 ? 1 : -1);
                activeLevel      = minecraft->getLevel(otherDimensionId);
            }

            MultiPlayerLevel* dimensionLevel = new MultiPlayerLevel(
                this,
                new LevelSettings(
                    packet->seed,
                    GameType::byId(packet->gameType),
                    false,
                    false,
                    packet->m_newSeaLevel,
                    packet->m_pLevelType,
                    packet->m_xzSize,
                    packet->m_hellScale
                ),
                packet->dimension,
                packet->difficulty
            );

            dimensionLevel->savedDataStorage = activeLevel->savedDataStorage;

            dimensionLevel->difficulty   = packet->difficulty; // 4J Added
            dimensionLevel->isClientSide = true;
            level                        = dimensionLevel;
            // 4J Stu - At time of writing ProfileManager.GetGamertag() does not
            // always return the correct name, if sign-ins are turned off while
            // the player signed in. Using the qnetPlayer instead. need to have
            // a level before create extra local player
            MultiPlayerLevel* levelpassedin = (MultiPlayerLevel*)level;
            player                          = minecraft->createExtraLocalPlayer(
                m_userIndex,
                networkPlayer->GetOnlineName(),
                m_userIndex,
                packet->dimension,
                this,
                levelpassedin
            );

            // need to have a player before the setlevel
            std::shared_ptr<MultiplayerLocalPlayer> lastPlayer =
                minecraft->player;
            minecraft->player = minecraft->localplayers[m_userIndex];
            minecraft->setLevel(level);
            minecraft->player = lastPlayer;
        } else {
            player = minecraft->createExtraLocalPlayer(
                m_userIndex,
                networkPlayer->GetOnlineName(),
                m_userIndex,
                packet->dimension,
                this
            );
        }


        // level->addClientConnection( this );
        player->dimension = packet->dimension;
        player->entityId  = packet->clientVersion;

        player->setPlayerIndex(packet->m_playerIndex);
        player->setCustomSkin(app.GetPlayerSkinId(m_userIndex));
        player->setCustomCape(app.GetPlayerCapeId(m_userIndex));


        BYTE networkSmallId = getSocket()->getSmallId();
        app.UpdatePlayerInfo(
            networkSmallId,
            packet->m_playerIndex,
            packet->m_uiGamePrivileges
        );
        player->setPlayerGamePrivilege(
            Player::ePlayerGamePrivilege_All,
            packet->m_uiGamePrivileges
        );

        // Assume all privileges are on, so that the first message we see only
        // indicates things that have been turned off
        unsigned int startingPrivileges = 0;
        Player::enableAllPlayerPrivileges(startingPrivileges, true);

        displayPrivilegeChanges(
            minecraft->localplayers[m_userIndex],
            startingPrivileges
        );
    }

    maxPlayers = packet->maxPlayers;

    // need to have a player before the setLocalCreativeMode
    std::shared_ptr<MultiplayerLocalPlayer> lastPlayer = minecraft->player;
    minecraft->player = minecraft->localplayers[m_userIndex];
    ((MultiPlayerGameMode*)minecraft->localgameModes[m_userIndex])
        ->setLocalMode(GameType::byId(packet->gameType));
    minecraft->player = lastPlayer;

    // make sure the UI offsets for this player are set correctly
    if (iUserID != -1) {
        ui.UpdateSelectedItemPos(iUserID);
    }

    TelemetryManager->RecordLevelStart(
        m_userIndex,
        eSen_FriendOrMatch_Playing_With_Invited_Friends,
        eSen_CompeteOrCoop_Coop_and_Competitive,
        Minecraft::GetInstance()->getLevel(packet->dimension)->difficulty,
        app.GetLocalPlayerCount(),
        g_NetworkManager.GetOnlinePlayerCount()
    );
}

void ClientConnection::handleAddEntity(
    std::shared_ptr<AddEntityPacket> packet
) {
    double                  x = packet->x / 32.0;
    double                  y = packet->y / 32.0;
    double                  z = packet->z / 32.0;
    std::shared_ptr<Entity> e;
    bool                    setRot = true;

    // 4J-PB - replacing this massive if nest with switch
    switch (packet->type) {
    case AddEntityPacket::MINECART_RIDEABLE:
        e = std::shared_ptr<Entity>(
            new Minecart(level, x, y, z, Minecart::RIDEABLE)
        );
        break;
    case AddEntityPacket::MINECART_CHEST:
        e = std::shared_ptr<Entity>(
            new Minecart(level, x, y, z, Minecart::CHEST)
        );
        break;

    case AddEntityPacket::MINECART_FURNACE:
        e = std::shared_ptr<Entity>(
            new Minecart(level, x, y, z, Minecart::FURNACE)
        );
        break;

    case AddEntityPacket::FISH_HOOK: {
        // 4J Stu - Brought forward from 1.4 to be able to drop XP from fishing
        std::shared_ptr<Entity> owner = getEntity(packet->data);

        // 4J - check all local players to find match
        if (owner == NULL) {
            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                if (minecraft->localplayers[i]) {
                    if (minecraft->localplayers[i]->entityId == packet->data) {
                        owner = minecraft->localplayers[i];
                        break;
                    }
                }
            }
        }
        std::shared_ptr<Player> player =
            std::dynamic_pointer_cast<Player>(owner);
        if (player != NULL) {
            std::shared_ptr<FishingHook> hook = std::shared_ptr<FishingHook>(
                new FishingHook(level, x, y, z, player)
            );
            e = hook;
            // 4J Stu - Move the player->fishing out of the ctor as we cannot
            // reference 'this'
            player->fishing = hook;
        }
        packet->data = 0;
    } break;
    case AddEntityPacket::ARROW:
        e = std::shared_ptr<Entity>(new Arrow(level, x, y, z));
        break;
    case AddEntityPacket::SNOWBALL:
        e = std::shared_ptr<Entity>(new Snowball(level, x, y, z));
        break;
    case AddEntityPacket::ITEM_FRAME: {
        int ix = (int)x;
        int iy = (int)y;
        int iz = (int)z;
        app.DebugPrintf(
            "ClientConnection ITEM_FRAME xyz %d,%d,%d\n",
            ix,
            iy,
            iz
        );
    }
        e = std::shared_ptr<Entity>(
            new ItemFrame(level, (int)x, (int)y, (int)z, packet->data)
        );
        packet->data = 0;
        setRot       = false;
        break;
    case AddEntityPacket::THROWN_ENDERPEARL:
        e = std::shared_ptr<Entity>(new ThrownEnderpearl(level, x, y, z));
        break;
    case AddEntityPacket::EYEOFENDERSIGNAL:
        e = std::shared_ptr<Entity>(new EyeOfEnderSignal(level, x, y, z));
        break;
    case AddEntityPacket::FIREBALL:
        e            = std::shared_ptr<Entity>(new Fireball(
            level,
            x,
            y,
            z,
            packet->xa / 8000.0,
            packet->ya / 8000.0,
            packet->za / 8000.0
        ));
        packet->data = 0;
        break;
    case AddEntityPacket::SMALL_FIREBALL:
        e            = std::shared_ptr<Entity>(new SmallFireball(
            level,
            x,
            y,
            z,
            packet->xa / 8000.0,
            packet->ya / 8000.0,
            packet->za / 8000.0
        ));
        packet->data = 0;
        break;
    case AddEntityPacket::DRAGON_FIRE_BALL:
        e            = std::shared_ptr<Entity>(new DragonFireball(
            level,
            x,
            y,
            z,
            packet->xa / 8000.0,
            packet->ya / 8000.0,
            packet->za / 8000.0
        ));
        packet->data = 0;
        break;
    case AddEntityPacket::EGG:
        e = std::shared_ptr<Entity>(new ThrownEgg(level, x, y, z));
        break;
    case AddEntityPacket::THROWN_POTION:
        e = std::shared_ptr<Entity>(
            new ThrownPotion(level, x, y, z, packet->data)
        );
        packet->data = 0;
        break;
    case AddEntityPacket::THROWN_EXPBOTTLE:
        e = std::shared_ptr<Entity>(new ThrownExpBottle(level, x, y, z));
        packet->data = 0;
        break;
    case AddEntityPacket::BOAT:
        e = std::shared_ptr<Entity>(new Boat(level, x, y, z));
        break;
    case AddEntityPacket::PRIMED_TNT:
        e = std::shared_ptr<Entity>(new PrimedTnt(level, x, y, z));
        break;
    case AddEntityPacket::ENDER_CRYSTAL:
        e = std::shared_ptr<Entity>(new EnderCrystal(level, x, y, z));
        break;
    case AddEntityPacket::ITEM:
        e = std::shared_ptr<Entity>(new ItemEntity(level, x, y, z));
        break;
    case AddEntityPacket::FALLING:
        e            = std::shared_ptr<Entity>(new FallingTile(
            level,
            x,
            y,
            z,
            packet->data & 0xFFFF,
            packet->data >> 16
        ));
        packet->data = 0;
        break;
    }
    /*   if (packet->type == AddEntityPacket::MINECART_RIDEABLE) e =
       std::shared_ptr<Entity>( new Minecart(level, x, y, z, Minecart::RIDEABLE)
       ); if (packet->type == AddEntityPacket::MINECART_CHEST) e =
       std::shared_ptr<Entity>( new Minecart(level, x, y, z, Minecart::CHEST) );
       if (packet->type == AddEntityPacket::MINECART_FURNACE) e =
       std::shared_ptr<Entity>( new Minecart(level, x, y, z, Minecart::FURNACE)
       ); if (packet->type == AddEntityPacket::FISH_HOOK)
       {
           // 4J Stu - Brought forward from 1.4 to be able to drop XP from
       fishing std::shared_ptr<Entity> owner = getEntity(packet->data);

           // 4J - check all local players to find match
           if( owner == NULL )
           {
               for( int i = 0; i < XUSER_MAX_COUNT; i++ )
               {
                   if( minecraft->localplayers[i] )
                   {
                       if( minecraft->localplayers[i]->entityId == packet->data
       )
                       {

                           owner = minecraft->localplayers[i];
                           break;
                       }
                   }
               }
           }
           std::shared_ptr<Player> player =
       std::dynamic_pointer_cast<Player>(owner); if (player != NULL)
           {
               std::shared_ptr<FishingHook> hook = std::shared_ptr<FishingHook>(
       new FishingHook(level, x, y, z, player) ); e = hook;
               // 4J Stu - Move the player->fishing out of the ctor as we cannot
       reference 'this' player->fishing = hook;
           }
           packet->data = 0;
       }

       if (packet->type == AddEntityPacket::ARROW) e = std::shared_ptr<Entity>(
       new Arrow(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::SNOWBALL) e = std::shared_ptr<Entity>( new
       Snowball(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::THROWN_ENDERPEARL) e = std::shared_ptr<Entity>( new
       ThrownEnderpearl(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::EYEOFENDERSIGNAL) e = std::shared_ptr<Entity>( new
       EyeOfEnderSignal(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::FIREBALL)
       {
           e = std::shared_ptr<Entity>( new Fireball(level, x, y, z, packet->xa
       / 8000.0, packet->ya / 8000.0, packet->za / 8000.0) ); packet->data = 0;
       }
       if (packet->type == AddEntityPacket::SMALL_FIREBALL)
       {
           e = std::shared_ptr<Entity>( new SmallFireball(level, x, y, z,
       packet->xa / 8000.0, packet->ya / 8000.0, packet->za / 8000.0) );
       packet->data = 0;
       }
       if (packet->type == AddEntityPacket::EGG) e = std::shared_ptr<Entity>(
       new ThrownEgg(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::THROWN_POTION)
       {
           e = std::shared_ptr<Entity>( new ThrownPotion(level, x, y, z,
       packet->data) ); packet->data = 0;
       }
       if (packet->type == AddEntityPacket::THROWN_EXPBOTTLE)
       {
           e = std::shared_ptr<Entity>( new ThrownExpBottle(level, x, y, z) );
           packet->data = 0;
       }
       if (packet->type == AddEntityPacket::BOAT) e = std::shared_ptr<Entity>(
       new Boat(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::PRIMED_TNT) e = std::shared_ptr<Entity>( new
       PrimedTnt(level, x, y, z) ); if (packet->type
       == AddEntityPacket::ENDER_CRYSTAL) e = std::shared_ptr<Entity>( new
       EnderCrystal(level, x, y, z) ); if (packet->type ==
       AddEntityPacket::FALLING_SAND) e = std::shared_ptr<Entity>( new
       FallingTile(level, x, y, z, Tile::sand->id) ); if (packet->type ==
       AddEntityPacket::FALLING_GRAVEL) e = std::shared_ptr<Entity>( new
       FallingTile(level, x, y, z, Tile::gravel->id) ); if (packet->type ==
       AddEntityPacket::FALLING_EGG) e = std::shared_ptr<Entity>( new
       FallingTile(level, x, y, z, Tile::dragonEgg_Id) );

       */

    if (e != NULL) {
        e->xp = packet->x;
        e->yp = packet->y;
        e->zp = packet->z;

        float yRot = packet->yRot * 360 / 256.0f;
        float xRot = packet->xRot * 360 / 256.0f;
        e->yRotp   = packet->yRot;
        e->xRotp   = packet->xRot;

        if (setRot) {
            e->yRot = 0.0f;
            e->xRot = 0.0f;
        }

        std::vector<std::shared_ptr<Entity>>* subEntities = e->getSubEntities();
        if (subEntities != NULL) {
            int offs = packet->id - e->entityId;
            // for (int i = 0; i < subEntities.length; i++)
            for (auto it = subEntities->begin(); it != subEntities->end();
                 ++it) {
                (*it)->entityId += offs;
                // subEntities[i].entityId += offs;
                // System.out.println(subEntities[i].entityId);
            }
        }

        // Note - not doing this move for frame, as the ctor for these objects
        // does some adjustments on the position based on direction to move the
        // object out slightly from what it is attached to, and this just
        // overwrites it
        if (packet->type != AddEntityPacket::ITEM_FRAME) {
            e->absMoveTo(x, y, z, yRot, xRot);
        }
        e->entityId = packet->id;
        level->putEntity(packet->id, e);

        if (packet->data > -1) // 4J - changed "no data" value to be -1, we can
                               // have a valid entity id of 0
        {
            if (packet->type == AddEntityPacket::ARROW) {
                std::shared_ptr<Entity> owner = getEntity(packet->data);

                // 4J - check all local players to find match
                if (owner == NULL) {
                    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                        if (minecraft->localplayers[i]) {
                            if (minecraft->localplayers[i]->entityId
                                == packet->data) {
                                owner = minecraft->localplayers[i];
                                break;
                            }
                        }
                    }
                }

                if (std::dynamic_pointer_cast<Mob>(owner) != NULL) {
                    std::dynamic_pointer_cast<Arrow>(e)->owner =
                        std::dynamic_pointer_cast<Mob>(owner);
                }
            }

            e->lerpMotion(
                packet->xa / 8000.0,
                packet->ya / 8000.0,
                packet->za / 8000.0
            );
        }
    }
}

void ClientConnection::handleAddExperienceOrb(
    std::shared_ptr<AddExperienceOrbPacket> packet
) {
    std::shared_ptr<Entity> e =
        std::shared_ptr<ExperienceOrb>(new ExperienceOrb(
            level,
            packet->x / 32.0,
            packet->y / 32.0,
            packet->z / 32.0,
            packet->value
        ));
    e->xp       = packet->x;
    e->yp       = packet->y;
    e->zp       = packet->z;
    e->yRot     = 0;
    e->xRot     = 0;
    e->entityId = packet->id;
    level->putEntity(packet->id, e);
}

void ClientConnection::handleAddGlobalEntity(
    std::shared_ptr<AddGlobalEntityPacket> packet
) {
    double                  x = packet->x / 32.0;
    double                  y = packet->y / 32.0;
    double                  z = packet->z / 32.0;
    std::shared_ptr<Entity> e; // = nullptr;
    if (packet->type == AddGlobalEntityPacket::LIGHTNING)
        e = std::shared_ptr<LightningBolt>(new LightningBolt(level, x, y, z));
    if (e != NULL) {
        e->xp       = packet->x;
        e->yp       = packet->y;
        e->zp       = packet->z;
        e->yRot     = 0;
        e->xRot     = 0;
        e->entityId = packet->id;
        level->addGlobalEntity(e);
    }
}

void ClientConnection::handleAddPainting(
    std::shared_ptr<AddPaintingPacket> packet
) {
    std::shared_ptr<Painting> painting = std::shared_ptr<Painting>(new Painting(
        level,
        packet->x,
        packet->y,
        packet->z,
        packet->dir,
        packet->motive
    ));
    level->putEntity(packet->id, painting);
}

void ClientConnection::handleSetEntityMotion(
    std::shared_ptr<SetEntityMotionPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    e->lerpMotion(
        packet->xa / 8000.0,
        packet->ya / 8000.0,
        packet->za / 8000.0
    );
}

void ClientConnection::handleSetEntityData(
    std::shared_ptr<SetEntityDataPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e != NULL && packet->getUnpackedData() != NULL) {
        e->getEntityData()->assignValues(packet->getUnpackedData());
    }
}

void ClientConnection::handleAddPlayer(
    std::shared_ptr<AddPlayerPacket> packet
) {
    // Some remote players could actually be local players that are already
    // added
    for (unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
        // need to use the XUID here
        PlayerUID playerXUIDOnline  = INVALID_XUID,
                  playerXUIDOffline = INVALID_XUID;
        ProfileManager.GetXUID(idx, &playerXUIDOnline, true);
        ProfileManager.GetXUID(idx, &playerXUIDOffline, false);
        if ((playerXUIDOnline != INVALID_XUID
             && ProfileManager.AreXUIDSEqual(playerXUIDOnline, packet->xuid))
            || (playerXUIDOffline != INVALID_XUID
                && ProfileManager
                       .AreXUIDSEqual(playerXUIDOffline, packet->xuid))) {
            app.DebugPrintf(
                "AddPlayerPacket received with XUID of local player\n"
            );
            return;
        }
    }

    double                        x      = packet->x / 32.0;
    double                        y      = packet->y / 32.0;
    double                        z      = packet->z / 32.0;
    float                         yRot   = packet->yRot * 360 / 256.0f;
    float                         xRot   = packet->xRot * 360 / 256.0f;
    std::shared_ptr<RemotePlayer> player = std::shared_ptr<RemotePlayer>(
        new RemotePlayer(minecraft->level, packet->name)
    );
    player->xo = player->xOld = player->xp = packet->x;
    player->yo = player->yOld = player->yp = packet->y;
    player->zo = player->zOld = player->zp = packet->z;
    player->xRotp                          = packet->xRot;
    player->yRotp                          = packet->yRot;
    player->yHeadRot                       = packet->yHeadRot * 360 / 256.0f;
    player->setXuid(packet->xuid);

#ifdef _DURANGO
    // On Durango request player display name from network manager
    INetworkPlayer* networkPlayer =
        g_NetworkManager.GetPlayerByXuid(player->getXuid());
    if (networkPlayer != NULL)
        player->displayName = networkPlayer->GetDisplayName();
#else
    // On all other platforms display name is just gamertag so don't check with
    // the network manager
    player->displayName = player->name;
#endif

    //	printf("\t\t\t\t%d: Add player\n",packet->id,packet->yRot);

    int item = packet->carriedItem;
    if (item == 0) {
        player->inventory->items[player->inventory->selected] =
            std::shared_ptr<ItemInstance>(); // NULL;
    } else {
        player->inventory->items[player->inventory->selected] =
            std::shared_ptr<ItemInstance>(new ItemInstance(item, 1, 0));
    }
    player->absMoveTo(x, y, z, yRot, xRot);

    player->setPlayerIndex(packet->m_playerIndex);
    player->setCustomSkin(packet->m_skinId);
    player->setCustomCape(packet->m_capeId);
    player->setPlayerGamePrivilege(
        Player::ePlayerGamePrivilege_All,
        packet->m_uiGamePrivileges
    );

    if (!player->customTextureUrl.empty()
        && player->customTextureUrl.substr(0, 3).compare(L"def") != 0
        && !app.IsFileInMemoryTextures(player->customTextureUrl)) {
        if (minecraft->addPendingClientTextureRequest(
                player->customTextureUrl
            )) {
            app.DebugPrintf(
                "Client sending TextureAndGeometryPacket to get custom skin "
                "%ls for player %ls\n",
                player->customTextureUrl.c_str(),
                player->name.c_str()
            );

            send(
                std::shared_ptr<TextureAndGeometryPacket>(
                    new TextureAndGeometryPacket(
                        player->customTextureUrl,
                        NULL,
                        0
                    )
                )
            );
        }
    } else if (!player->customTextureUrl.empty()
               && app.IsFileInMemoryTextures(player->customTextureUrl)) {
        // Update the ref count on the memory texture data
        app.AddMemoryTextureFile(player->customTextureUrl, NULL, 0);
    }

    app.DebugPrintf(
        "Custom skin for player %ls is %ls\n",
        player->name.c_str(),
        player->customTextureUrl.c_str()
    );

    if (!player->customTextureUrl2.empty()
        && player->customTextureUrl2.substr(0, 3).compare(L"def") != 0
        && !app.IsFileInMemoryTextures(player->customTextureUrl2)) {
        if (minecraft->addPendingClientTextureRequest(
                player->customTextureUrl2
            )) {
            app.DebugPrintf(
                "Client sending texture packet to get custom cape %ls for "
                "player %ls\n",
                player->customTextureUrl2.c_str(),
                player->name.c_str()
            );
            send(
                std::shared_ptr<TexturePacket>(
                    new TexturePacket(player->customTextureUrl2, NULL, 0)
                )
            );
        }
    } else if (!player->customTextureUrl2.empty()
               && app.IsFileInMemoryTextures(player->customTextureUrl2)) {
        // Update the ref count on the memory texture data
        app.AddMemoryTextureFile(player->customTextureUrl2, NULL, 0);
    }

    app.DebugPrintf(
        "Custom cape for player %ls is %ls\n",
        player->name.c_str(),
        player->customTextureUrl2.c_str()
    );

    level->putEntity(packet->id, player);

    std::vector<std::shared_ptr<SynchedEntityData::DataItem>>* unpackedData =
        packet->getUnpackedData();
    if (unpackedData != NULL) {
        player->getEntityData()->assignValues(unpackedData);
    }
}

void ClientConnection::handleTeleportEntity(
    std::shared_ptr<TeleportEntityPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    e->xp    = packet->x;
    e->yp    = packet->y;
    e->zp    = packet->z;
    double x = e->xp / 32.0;
    double y = e->yp / 32.0 + 1 / 64.0f;
    double z = e->zp / 32.0;
    // 4J - make sure xRot stays within -90 -> 90 range
    int ixRot = packet->xRot;
    if (ixRot >= 128) ixRot -= 256;
    float yRot = packet->yRot * 360 / 256.0f;
    float xRot = ixRot * 360 / 256.0f;
    e->yRotp   = packet->yRot;
    e->xRotp   = ixRot;

    //	printf("\t\t\t\t%d: Teleport to %d (lerp to
    //%f)\n",packet->id,packet->yRot,yRot);
    e->lerpTo(x, y, z, yRot, xRot, 3);
}

void ClientConnection::handleMoveEntity(
    std::shared_ptr<MoveEntityPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    e->xp    += packet->xa;
    e->yp    += packet->ya;
    e->zp    += packet->za;
    double x  = e->xp / 32.0;
    // 4J - The original code did not add the 1/64.0f like the teleport above
    // did, which caused minecarts to fall through the ground
    double y = e->yp / 32.0 + 1 / 64.0f;
    double z = e->zp / 32.0;
    // 4J - have changed rotation to be relative here too
    e->yRotp   += packet->yRot;
    e->xRotp   += packet->xRot;
    float yRot  = (e->yRotp * 360) / 256.0f;
    float xRot  = (e->xRotp * 360) / 256.0f;
    //    float yRot = packet->hasRot ? packet->yRot * 360 / 256.0f : e->yRot;
    //    float xRot = packet->hasRot ? packet->xRot * 360 / 256.0f : e->xRot;
    e->lerpTo(x, y, z, yRot, xRot, 3);
}

void ClientConnection::handleRotateMob(
    std::shared_ptr<RotateHeadPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    float yHeadRot = packet->yHeadRot * 360 / 256.f;
    e->setYHeadRot(yHeadRot);
}

void ClientConnection::handleMoveEntitySmall(
    std::shared_ptr<MoveEntityPacketSmall> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    e->xp    += packet->xa;
    e->yp    += packet->ya;
    e->zp    += packet->za;
    double x  = e->xp / 32.0;
    // 4J - The original code did not add the 1/64.0f like the teleport above
    // did, which caused minecarts to fall through the ground
    double y = e->yp / 32.0 + 1 / 64.0f;
    double z = e->zp / 32.0;
    // 4J - have changed rotation to be relative here too
    e->yRotp   += packet->yRot;
    e->xRotp   += packet->xRot;
    float yRot  = (e->yRotp * 360) / 256.0f;
    float xRot  = (e->xRotp * 360) / 256.0f;
    //    float yRot = packet->hasRot ? packet->yRot * 360 / 256.0f : e->yRot;
    //    float xRot = packet->hasRot ? packet->xRot * 360 / 256.0f : e->xRot;
    e->lerpTo(x, y, z, yRot, xRot, 3);
}

void ClientConnection::handleRemoveEntity(
    std::shared_ptr<RemoveEntitiesPacket> packet
) {
    for (int i = 0; i < packet->ids.length; i++) {
        level->removeEntity(packet->ids[i]);
    }
}

void ClientConnection::handleMovePlayer(
    std::shared_ptr<MovePlayerPacket> packet
) {
    std::shared_ptr<Player> player =
        minecraft->localplayers[m_userIndex]; // minecraft->player;

    double x    = player->x;
    double y    = player->y;
    double z    = player->z;
    float  yRot = player->yRot;
    float  xRot = player->xRot;

    if (packet->hasPos) {
        x = packet->x;
        y = packet->y;
        z = packet->z;
    }
    if (packet->hasRot) {
        yRot = packet->yRot;
        xRot = packet->xRot;
    }

    player->ySlideOffset = 0;
    player->xd = player->yd = player->zd = 0;
    player->absMoveTo(x, y, z, yRot, xRot);
    packet->x     = player->x;
    packet->y     = player->bb->y0;
    packet->z     = player->z;
    packet->yView = player->y;
    connection->send(packet);
    if (!started) {
        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCConnected * 100) / (eCCConnected)
            );
        }
        player->xo = player->x;
        player->yo = player->y;
        player->zo = player->z;
        // 4J - added setting xOld/yOld/zOld here too, as otherwise at the start
        // of the game we interpolate the player position from the origin to
        // wherever its first position really is
        player->xOld = player->x;
        player->yOld = player->y;
        player->zOld = player->z;

        started = true;
        minecraft->setScreen(NULL);

        // Fix for #105852 - TU12: Content: Gameplay: Local splitscreen Players
        // are spawned at incorrect places after re-joining previously saved and
        // loaded "Mass Effect World". Move this check from
        // Minecraft::createExtraLocalPlayer 4J-PB - can't call this when this
        // function is called from the qnet thread (GetGameStarted will be
        // false)
        if (app.GetGameStarted()) {
            ui.CloseUIScenes(m_userIndex);
        }
    }
}

// 4J Added
void ClientConnection::handleChunkVisibilityArea(
    std::shared_ptr<ChunkVisibilityAreaPacket> packet
) {
    for (int z = packet->m_minZ; z <= packet->m_maxZ; ++z)
        for (int x = packet->m_minX; x <= packet->m_maxX; ++x)
            level->setChunkVisible(x, z, true);
}

void ClientConnection::handleChunkVisibility(
    std::shared_ptr<ChunkVisibilityPacket> packet
) {
    level->setChunkVisible(packet->x, packet->z, packet->visible);
}

void ClientConnection::handleChunkTilesUpdate(
    std::shared_ptr<ChunkTilesUpdatePacket> packet
) {
    // 4J - changed to encode level in packet
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        PIXBeginNamedEvent(0, "Handle chunk tiles update");
        LevelChunk* lc = dimensionLevel->getChunk(packet->xc, packet->zc);
        int         xo = packet->xc * 16;
        int         zo = packet->zc * 16;
        // 4J Stu - Unshare before we make any changes incase the server is
        // already another step ahead of us Fix for #7904 - Gameplay: Players
        // can dupe torches by throwing them repeatedly into water. This is
        // quite expensive to do, so only consider unsharing if this tile
        // setting is going to actually change something
        bool forcedUnshare = false;
        for (int i = 0; i < packet->count; i++) {
            int pos  = packet->positions[i];
            int tile = packet->blocks[i] & 0xff;
            int data = packet->data[i];


            int x = (pos >> 12) & 15;
            int z = (pos >> 8) & 15;
            int y = ((pos) & 255);

            // If this is going to actually change a tile, we'll need to unshare
            int prevTile = lc->getTile(x, y, z);
            if ((tile != prevTile && !forcedUnshare)) {
                PIXBeginNamedEvent(0, "Chunk data unsharing\n");
                dimensionLevel->unshareChunkAt(xo, zo);
                PIXEndNamedEvent();
                forcedUnshare = true;
            }

            // 4J - Changes now that lighting is done at the client side of
            // things... Note - the java version now calls the doSetTileAndData
            // method from the level here rather than the levelchunk, which
            // ultimately ends up calling checkLight for the altered tile. For
            // us this doesn't always work as when sharing tile data between a
            // local server & client, the tile might not be considered to be
            // being changed on the client as the server already has changed the
            // shared data, and so the checkLight doesn't happen. Hence doing an
            // explicit checkLight here instead.
            lc->setTileAndData(x, y, z, tile, data);
            dimensionLevel->checkLight(x + xo, y, z + zo);

            dimensionLevel
                ->clearResetRegion(x + xo, y, z + zo, x + xo, y, z + zo);

            // Don't bother setting this to dirty if it isn't going to visually
            // change - we get a lot of water changing from static to dynamic
            // for instance
            if (!(((prevTile == Tile::water_Id) && (tile == Tile::calmWater_Id))
                  || ((prevTile == Tile::calmWater_Id)
                      && (tile == Tile::water_Id))
                  || ((prevTile == Tile::lava_Id)
                      && (tile == Tile::calmLava_Id))
                  || ((prevTile == Tile::calmLava_Id)
                      && (tile == Tile::calmLava_Id))
                  || ((prevTile == Tile::calmLava_Id)
                      && (tile == Tile::lava_Id)))) {
                dimensionLevel
                    ->setTilesDirty(x + xo, y, z + zo, x + xo, y, z + zo);
            }

            // 4J - remove any tite entities in this region which are associated
            // with a tile that is now no longer a tile entity. Without doing
            // this we end up with stray tile entities kicking round, which
            // leads to a bug where chests can't be properly placed again in a
            // location after (say) a chest being removed by TNT
            dimensionLevel->removeUnusedTileEntitiesInRegion(
                xo + x,
                y,
                zo + z,
                xo + x + 1,
                y + 1,
                zo + z + 1
            );
        }
        PIXBeginNamedEvent(0, "Chunk data sharing\n");
        dimensionLevel->shareChunkAt(
            xo,
            zo
        ); // 4J - added - only shares if chunks are same on server & client
        PIXEndNamedEvent();

        PIXEndNamedEvent();
    }
}

void ClientConnection::handleBlockRegionUpdate(
    std::shared_ptr<BlockRegionUpdatePacket> packet
) {
    // 4J - changed to encode level in packet
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        PIXBeginNamedEvent(0, "Handle block region update");

        int y1 = packet->y + packet->ys;
        if (packet->bIsFullChunk) {
            y1 = Level::maxBuildHeight;
            if (packet->buffer.length > 0)
                LevelChunk::reorderBlocksAndDataToXZY(
                    packet->y,
                    packet->xs,
                    packet->ys,
                    packet->zs,
                    &packet->buffer
                );
        }
        dimensionLevel->clearResetRegion(
            packet->x,
            packet->y,
            packet->z,
            packet->x + packet->xs - 1,
            y1 - 1,
            packet->z + packet->zs - 1
        );

        // Only full chunks send lighting information now - added flag to end of
        // this call
        dimensionLevel->setBlocksAndData(
            packet->x,
            packet->y,
            packet->z,
            packet->xs,
            packet->ys,
            packet->zs,
            packet->buffer,
            packet->bIsFullChunk
        );

        //		OutputDebugString("END BRU\n");

        // 4J - remove any tite entities in this region which are associated
        // with a tile that is now no longer a tile entity. Without doing this
        // we end up with stray tile entities kicking round, which leads to a
        // bug where chests can't be properly placed again in a location after
        // (say) a chest being removed by TNT
        dimensionLevel->removeUnusedTileEntitiesInRegion(
            packet->x,
            packet->y,
            packet->z,
            packet->x + packet->xs,
            y1,
            packet->z + packet->zs
        );

        // If this is a full packet for a chunk, make sure that the cache now
        // considers that it has data for this chunk - this is used to determine
        // whether to bother rendering mobs or not, so we don't have them in
        // crazy positions before the data is there
        if (packet->bIsFullChunk) {
            dimensionLevel->dataReceivedForChunk(
                packet->x >> 4,
                packet->z >> 4
            );
        }
        PIXEndNamedEvent();
    }
}

void ClientConnection::handleTileUpdate(
    std::shared_ptr<TileUpdatePacket> packet
) {
    // 4J added - using a block of 255 to signify that this is a packet for
    // destroying a tile, where we need to inform the level renderer that we are
    // about to do so. This is used in creative mode as the point where a tile
    // is first destroyed at the client end of things. Packets formed like this
    // are potentially sent from ServerPlayerGameMode::destroyBlock
    bool destroyTilePacket = false;
    if (packet->block == 255) {
        packet->block     = 0;
        destroyTilePacket = true;
    }
    // 4J - changed to encode level in packet
    MultiPlayerLevel* dimensionLevel =
        (MultiPlayerLevel*)minecraft->levels[packet->levelIdx];
    if (dimensionLevel) {
        PIXBeginNamedEvent(0, "Handle tile update");

        if (g_NetworkManager.IsHost()) {
            // 4J Stu - Unshare before we make any changes incase the server is
            // already another step ahead of us Fix for #7904 - Gameplay:
            // Players can dupe torches by throwing them repeatedly into water.
            // This is quite expensive to do, so only consider unsharing if this
            // tile setting is going to actually change something
            int prevTile =
                dimensionLevel->getTile(packet->x, packet->y, packet->z);
            int prevData =
                dimensionLevel->getData(packet->x, packet->y, packet->z);
            if (packet->block != prevTile || packet->data != prevData) {
                PIXBeginNamedEvent(0, "Chunk data unsharing\n");
                dimensionLevel->unshareChunkAt(packet->x, packet->z);
                PIXEndNamedEvent();
            }
        }

        // 4J - In creative mode, we don't update the tile locally then get it
        // confirmed by the server - the first point that we know we are about
        // to destroy a tile is here. Let the rendering side of thing know so we
        // can synchronise collision with async render data upates.
        if (destroyTilePacket) {
            minecraft->levelRenderer->destroyedTileManager->destroyingTileAt(
                dimensionLevel,
                packet->x,
                packet->y,
                packet->z
            );
        }

        PIXBeginNamedEvent(0, "Setting data\n");
        bool tileWasSet = dimensionLevel->doSetTileAndData(
            packet->x,
            packet->y,
            packet->z,
            packet->block,
            packet->data
        );

        PIXEndNamedEvent();

        // 4J - remove any tite entities in this region which are associated
        // with a tile that is now no longer a tile entity. Without doing this
        // we end up with stray tile entities kicking round, which leads to a
        // bug where chests can't be properly placed again in a location after
        // (say) a chest being removed by TNT
        dimensionLevel->removeUnusedTileEntitiesInRegion(
            packet->x,
            packet->y,
            packet->z,
            packet->x + 1,
            packet->y + 1,
            packet->z + 1
        );

        PIXBeginNamedEvent(0, "Sharing data\n");
        dimensionLevel->shareChunkAt(
            packet->x,
            packet->z
        ); // 4J - added - only shares if chunks are same on server & client
        PIXEndNamedEvent();

        PIXEndNamedEvent();
    }
}

void ClientConnection::handleDisconnect(
    std::shared_ptr<DisconnectPacket> packet
) {
    connection->close(DisconnectPacket::eDisconnect_Kicked);
    done = true;

    Minecraft* pMinecraft = Minecraft::GetInstance();
    pMinecraft->connectionDisconnected(m_userIndex, packet->reason);
    app.SetDisconnectReason(packet->reason);

    app.SetAction(m_userIndex, eAppAction_ExitWorld, (void*)TRUE);
    // minecraft->setLevel(NULL);
    // minecraft->setScreen(new DisconnectedScreen(L"disconnect.disconnected",
    // L"disconnect.genericReason", &packet->reason));
}

void ClientConnection::onDisconnect(
    DisconnectPacket::eDisconnectReason reason,
    void*                               reasonObjects
) {
    if (done) return;
    done = true;

    Minecraft* pMinecraft = Minecraft::GetInstance();
    pMinecraft->connectionDisconnected(m_userIndex, reason);

    // 4J Stu - TU-1 hotfix
    // Fix for #13191 - The host of a game can get a message informing them that
    // the connection to the server has been lost In the (now unlikely) event
    // that the host connections times out, allow the player to save their game
    if (g_NetworkManager.IsHost()
        && (reason == DisconnectPacket::eDisconnect_TimeOut
            || reason == DisconnectPacket::eDisconnect_Overflow)
        && m_userIndex == ProfileManager.GetPrimaryPad()
        && !MinecraftServer::saveOnExitAnswered()) {
        UINT uiIDA[1];
        uiIDA[0] = IDS_CONFIRM_OK;
        ui.RequestMessageBox(
            IDS_EXITING_GAME,
            IDS_GENERIC_ERROR,
            uiIDA,
            1,
            ProfileManager.GetPrimaryPad(),
            &ClientConnection::HostDisconnectReturned,
            NULL,
            app.GetStringTable()
        );
    } else {
        app.SetAction(m_userIndex, eAppAction_ExitWorld, (void*)TRUE);
    }

    // minecraft->setLevel(NULL);
    // minecraft->setScreen(new DisconnectedScreen(L"disconnect.lost", reason,
    // reasonObjects));
}

void ClientConnection::sendAndDisconnect(std::shared_ptr<Packet> packet) {
    if (done) return;
    connection->send(packet);
    connection->sendAndQuit();
}

void ClientConnection::send(std::shared_ptr<Packet> packet) {
    if (done) return;
    connection->send(packet);
}

void ClientConnection::handleTakeItemEntity(
    std::shared_ptr<TakeItemEntityPacket> packet
) {
    std::shared_ptr<Entity> from = getEntity(packet->itemId);
    std::shared_ptr<Mob>    to =
        std::dynamic_pointer_cast<Mob>(getEntity(packet->playerId));

    // 4J - the original game could assume that if getEntity didn't find the
    // player, it must be the local player. We need to search all local players
    bool isLocalPlayer = false;
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (minecraft->localplayers[i]) {
            if (minecraft->localplayers[i]->entityId == packet->playerId) {
                isLocalPlayer = true;
                to            = minecraft->localplayers[i];
                break;
            }
        }
    }

    if (to == NULL) {
        // Don't know if this should ever really happen, but seems safest to try
        // and remove the entity that has been collected even if we can't create
        // a particle as we don't know what really collected it
        level->removeEntity(packet->itemId);
        return;
    }

    if (from != NULL) {
        // If this is a local player, then we only want to do processing for it
        // if this connection is associated with the player it is for. In
        // particular, we don't want to remove the item entity until we are
        // processing it for the right connection, or else we won't have a valid
        // "from" reference if we've already removed the item for an earlier
        // processed connection
        if (isLocalPlayer) {
            std::shared_ptr<LocalPlayer> player =
                std::dynamic_pointer_cast<LocalPlayer>(to);

            // 4J Stu - Fix for #10213 - UI: Local clients cannot progress
            // through the tutorial normally. We only send this packet once if
            // many local players can see the event, so make sure we update the
            // tutorial for the player that actually picked up the item
            int playerPad = player->GetXboxPad();

            if (minecraft->localgameModes[playerPad] != NULL) {
                // 4J-PB - add in the XP orb sound
                if (from->GetType() == eTYPE_EXPERIENCEORB) {
                    float fPitch =
                        ((random->nextFloat() - random->nextFloat()) * 0.7f
                         + 1.0f)
                        * 2.0f;
                    app.DebugPrintf("XP Orb with pitch %f\n", fPitch);
                    level->playSound(from, eSoundType_RANDOM_ORB, 0.2f, fPitch);
                } else {
                    level->playSound(
                        from,
                        eSoundType_RANDOM_POP,
                        0.2f,
                        ((random->nextFloat() - random->nextFloat()) * 0.7f
                         + 1.0f)
                            * 2.0f
                    );
                }

                minecraft->particleEngine->add(
                    std::shared_ptr<TakeAnimationParticle>(
                        new TakeAnimationParticle(
                            minecraft->level,
                            from,
                            to,
                            -0.5f
                        )
                    )
                );
                level->removeEntity(packet->itemId);
            } else {
                // Don't know if this should ever really happen, but seems
                // safest to try and remove the entity that has been collected
                // even if it somehow isn't an itementity
                level->removeEntity(packet->itemId);
            }
        } else {
            level->playSound(
                from,
                eSoundType_RANDOM_POP,
                0.2f,
                ((random->nextFloat() - random->nextFloat()) * 0.7f + 1.0f)
                    * 2.0f
            );
            minecraft->particleEngine->add(
                std::shared_ptr<TakeAnimationParticle>(
                    new TakeAnimationParticle(minecraft->level, from, to, -0.5f)
                )
            );
            level->removeEntity(packet->itemId);
        }
    }
}

void ClientConnection::handleChat(std::shared_ptr<ChatPacket> packet) {
    std::wstring message;
    int          iPos;
    bool         displayOnGui = true;

    std::wstring playerDisplayName = L"";
    std::wstring sourceDisplayName = L"";

    // On platforms other than Xbox One this just sets display name to gamertag
    if (packet->m_stringArgs.size() >= 1)
        playerDisplayName = GetDisplayNameByGamertag(packet->m_stringArgs[0]);
    if (packet->m_stringArgs.size() >= 2)
        sourceDisplayName = GetDisplayNameByGamertag(packet->m_stringArgs[1]);

    switch (packet->m_messageType) {
    case ChatPacket::e_ChatBedOccupied:
        message = app.GetString(IDS_TILE_BED_OCCUPIED);
        break;
    case ChatPacket::e_ChatBedNoSleep:
        message = app.GetString(IDS_TILE_BED_NO_SLEEP);
        break;
    case ChatPacket::e_ChatBedNotValid:
        message = app.GetString(IDS_TILE_BED_NOT_VALID);
        break;
    case ChatPacket::e_ChatBedNotSafe:
        message = app.GetString(IDS_TILE_BED_NOTSAFE);
        break;
    case ChatPacket::e_ChatBedPlayerSleep:
        message = app.GetString(IDS_TILE_BED_PLAYERSLEEP);
        iPos    = message.find(L"%s");
        message.replace(iPos, 2, playerDisplayName);
        break;
    case ChatPacket::e_ChatBedMeSleep:
        message = app.GetString(IDS_TILE_BED_MESLEEP);
        break;
    case ChatPacket::e_ChatPlayerJoinedGame:
        message = app.GetString(IDS_PLAYER_JOINED);
        iPos    = message.find(L"%s");
        message.replace(iPos, 2, playerDisplayName);
        break;
    case ChatPacket::e_ChatPlayerLeftGame:
        message = app.GetString(IDS_PLAYER_LEFT);
        iPos    = message.find(L"%s");
        message.replace(iPos, 2, playerDisplayName);
        break;
    case ChatPacket::e_ChatPlayerKickedFromGame:
        message = app.GetString(IDS_PLAYER_KICKED);
        iPos    = message.find(L"%s");
        message.replace(iPos, 2, playerDisplayName);
        break;
    case ChatPacket::e_ChatCannotPlaceLava:
        displayOnGui = false;
        app.SetGlobalXuiAction(eAppAction_DisplayLavaMessage);
        break;
    case ChatPacket::e_ChatDeathInFire:
        message = app.GetString(IDS_DEATH_INFIRE);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathOnFire:
        message = app.GetString(IDS_DEATH_ONFIRE);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathLava:
        message = app.GetString(IDS_DEATH_LAVA);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathInWall:
        message = app.GetString(IDS_DEATH_INWALL);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathDrown:
        message = app.GetString(IDS_DEATH_DROWN);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathStarve:
        message = app.GetString(IDS_DEATH_STARVE);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathCactus:
        message = app.GetString(IDS_DEATH_CACTUS);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathFall:
        message = app.GetString(IDS_DEATH_FALL);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathOutOfWorld:
        message = app.GetString(IDS_DEATH_OUTOFWORLD);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathGeneric:
        message = app.GetString(IDS_DEATH_GENERIC);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathExplosion:
        message = app.GetString(IDS_DEATH_EXPLOSION);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathMagic:
        message = app.GetString(IDS_DEATH_MAGIC);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathAnvil:
        message = app.GetString(IDS_DEATH_FALLING_ANVIL);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathFallingBlock:
        message = app.GetString(IDS_DEATH_FALLING_TILE);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathDragonBreath:
        message = app.GetString(IDS_DEATH_DRAGON_BREATH);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatDeathMob:
        message = app.GetString(IDS_DEATH_MOB);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        message = replaceAll(
            message,
            L"{*SOURCE*}",
            app.getEntityName((eINSTANCEOF)packet->m_intArgs[0])
        );
        break;
    case ChatPacket::e_ChatDeathPlayer:
        message = app.GetString(IDS_DEATH_PLAYER);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        message = replaceAll(message, L"{*SOURCE*}", sourceDisplayName);
        break;
    case ChatPacket::e_ChatDeathArrow:
        message = app.GetString(IDS_DEATH_ARROW);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        if (packet->m_intArgs[0] == eTYPE_SERVERPLAYER) {
            message = replaceAll(message, L"{*SOURCE*}", sourceDisplayName);
        } else {
            message = replaceAll(
                message,
                L"{*SOURCE*}",
                app.getEntityName((eINSTANCEOF)packet->m_intArgs[0])
            );
        }
        break;
    case ChatPacket::e_ChatDeathFireball:
        message = app.GetString(IDS_DEATH_FIREBALL);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        if (packet->m_intArgs[0] == eTYPE_SERVERPLAYER) {
            message =
                replaceAll(message, L"{*SOURCE*}", packet->m_stringArgs[1]);
        } else {
            message = replaceAll(
                message,
                L"{*SOURCE*}",
                app.getEntityName((eINSTANCEOF)packet->m_intArgs[0])
            );
        }
        break;
    case ChatPacket::e_ChatDeathThrown:
        message = app.GetString(IDS_DEATH_THROWN);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        if (packet->m_intArgs[0] == eTYPE_SERVERPLAYER) {
            message = replaceAll(message, L"{*SOURCE*}", sourceDisplayName);
        } else {
            message = replaceAll(
                message,
                L"{*SOURCE*}",
                app.getEntityName((eINSTANCEOF)packet->m_intArgs[0])
            );
        }
        break;
    case ChatPacket::e_ChatDeathIndirectMagic:
        message = app.GetString(IDS_DEATH_INDIRECT_MAGIC);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        if (packet->m_intArgs[0] == eTYPE_SERVERPLAYER) {
            message = replaceAll(message, L"{*SOURCE*}", sourceDisplayName);
        } else {
            message = replaceAll(
                message,
                L"{*SOURCE*}",
                app.getEntityName((eINSTANCEOF)packet->m_intArgs[0])
            );
        }
        break;
    case ChatPacket::e_ChatDeathThorns:
        message = app.GetString(IDS_DEATH_THORNS);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        if (packet->m_intArgs[0] == eTYPE_SERVERPLAYER) {
            message = replaceAll(message, L"{*SOURCE*}", sourceDisplayName);
        } else {
            message = replaceAll(
                message,
                L"{*SOURCE*}",
                app.getEntityName((eINSTANCEOF)packet->m_intArgs[0])
            );
        }
        break;
    case ChatPacket::e_ChatPlayerEnteredEnd:
        message = app.GetString(IDS_PLAYER_ENTERED_END);
        iPos    = message.find(L"%s");
        message.replace(iPos, 2, playerDisplayName);
        break;
    case ChatPacket::e_ChatPlayerLeftEnd:
        message = app.GetString(IDS_PLAYER_LEFT_END);
        iPos    = message.find(L"%s");
        message.replace(iPos, 2, playerDisplayName);
        break;

    case ChatPacket::e_ChatPlayerMaxEnemies:
        message = app.GetString(IDS_MAX_ENEMIES_SPAWNED);
        break;
        // Spawn eggs
    case ChatPacket::e_ChatPlayerMaxVillagers:
        message = app.GetString(IDS_MAX_VILLAGERS_SPAWNED);
        break;
    case ChatPacket::e_ChatPlayerMaxPigsSheepCows:
        message = app.GetString(IDS_MAX_PIGS_SHEEP_COWS_CATS_SPAWNED);
        break;
    case ChatPacket::e_ChatPlayerMaxChickens:
        message = app.GetString(IDS_MAX_CHICKENS_SPAWNED);
        break;
    case ChatPacket::e_ChatPlayerMaxSquid:
        message = app.GetString(IDS_MAX_SQUID_SPAWNED);
        break;
    case ChatPacket::e_ChatPlayerMaxMooshrooms:
        message = app.GetString(IDS_MAX_MOOSHROOMS_SPAWNED);
        break;
    case ChatPacket::e_ChatPlayerMaxWolves:
        message = app.GetString(IDS_MAX_WOLVES_SPAWNED);
        break;

        // Breeding
    case ChatPacket::e_ChatPlayerMaxBredPigsSheepCows:
        message = app.GetString(IDS_MAX_PIGS_SHEEP_COWS_CATS_BRED);
        break;
    case ChatPacket::e_ChatPlayerMaxBredChickens:
        message = app.GetString(IDS_MAX_CHICKENS_BRED);
        break;
    case ChatPacket::e_ChatPlayerMaxBredMooshrooms:
        message = app.GetString(IDS_MAX_MUSHROOMCOWS_BRED);
        break;

    case ChatPacket::e_ChatPlayerMaxBredWolves:
        message = app.GetString(IDS_MAX_WOLVES_BRED);
        break;

        // can't shear the mooshroom
    case ChatPacket::e_ChatPlayerCantShearMooshroom:
        message = app.GetString(IDS_CANT_SHEAR_MOOSHROOM);
        break;

        // Paintings/Item Frames
    case ChatPacket::e_ChatPlayerMaxHangingEntities:
        message = app.GetString(IDS_MAX_HANGINGENTITIES);
        break;
        // Enemy spawn eggs in peaceful
    case ChatPacket::e_ChatPlayerCantSpawnInPeaceful:
        message = app.GetString(IDS_CANT_SPAWN_IN_PEACEFUL);
        break;

        // Enemy spawn eggs in peaceful
    case ChatPacket::e_ChatPlayerMaxBoats:
        message = app.GetString(IDS_MAX_BOATS);
        break;

    case ChatPacket::e_ChatCommandTeleportSuccess:
        message = app.GetString(IDS_COMMAND_TELEPORT_SUCCESS);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        if (packet->m_intArgs[0] == eTYPE_SERVERPLAYER) {
            message = replaceAll(
                message,
                L"{*DESTINATION*}",
                packet->m_stringArgs[1]
            );
        } else {
            message = replaceAll(
                message,
                L"{*DESTINATION*}",
                app.getEntityName((eINSTANCEOF)packet->m_intArgs[0])
            );
        }
        break;
    case ChatPacket::e_ChatCommandTeleportMe:
        message = app.GetString(IDS_COMMAND_TELEPORT_ME);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;
    case ChatPacket::e_ChatCommandTeleportToMe:
        message = app.GetString(IDS_COMMAND_TELEPORT_TO_ME);
        message = replaceAll(message, L"{*PLAYER*}", playerDisplayName);
        break;

    default:
        message = playerDisplayName;
        break;
    }

    // flag that a message is a death message
    bool bIsDeathMessage =
        (packet->m_messageType >= ChatPacket::e_ChatDeathInFire)
        && (packet->m_messageType <= ChatPacket::e_ChatDeathDragonBreath);

    if (displayOnGui)
        minecraft->gui->addMessage(message, m_userIndex, bIsDeathMessage);
}

void ClientConnection::handleAnimate(std::shared_ptr<AnimatePacket> packet) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    if (packet->action == AnimatePacket::SWING) {
        std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e);
        if (player != NULL) player->swing();
    } else if (packet->action == AnimatePacket::HURT) {
        e->animateHurt();
    } else if (packet->action == AnimatePacket::WAKE_UP) {
        std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e);
        if (player != NULL) player->stopSleepInBed(false, false, false);
    } else if (packet->action == AnimatePacket::RESPAWN) {
    } else if (packet->action == AnimatePacket::CRITICAL_HIT) {
        std::shared_ptr<CritParticle> critParticle =
            std::shared_ptr<CritParticle>(
                new CritParticle(minecraft->level, e)
            );
        critParticle->CritParticlePostConstructor();
        minecraft->particleEngine->add(critParticle);
    } else if (packet->action == AnimatePacket::MAGIC_CRITICAL_HIT) {
        std::shared_ptr<CritParticle> critParticle =
            std::shared_ptr<CritParticle>(
                new CritParticle(minecraft->level, e, eParticleType_magicCrit)
            );
        critParticle->CritParticlePostConstructor();
        minecraft->particleEngine->add(critParticle);
    } else if (packet->action == AnimatePacket::EAT
               && std::dynamic_pointer_cast<RemotePlayer>(e) != NULL) {
    }
}

void ClientConnection::handleEntityActionAtPosition(
    std::shared_ptr<EntityActionAtPositionPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    if (packet->action == EntityActionAtPositionPacket::START_SLEEP) {
        std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e);
        player->startSleepInBed(packet->x, packet->y, packet->z);

        if (player == minecraft->localplayers[m_userIndex]) {
            TelemetryManager->RecordEnemyKilledOrOvercome(
                m_userIndex,
                0,
                player->y,
                0,
                0,
                0,
                0,
                eTelemetryInGame_UseBed
            );
        }
    }
}

void ClientConnection::handlePreLogin(std::shared_ptr<PreLoginPacket> packet) {
//	printf("Client: handlePreLogin\n");
#if 1
    // 4J - Check that we can play with all the players already in the game who
    // have Friends-Only UGC set
    BOOL canPlay                   = TRUE;
    BOOL canPlayLocal              = TRUE;
    BOOL isAtLeastOneFriend        = g_NetworkManager.IsHost();
    BOOL isFriendsWithHost         = TRUE;
    BOOL cantPlayContentRestricted = FALSE;

    if (!g_NetworkManager.IsHost()) {
        // set the game host settings
        app.SetGameHostOption(eGameHostOption_All, packet->m_serverSettings);

        // 4J-PB - if we go straight in from the menus via an invite, we won't
        // have the DLC info
        if (app.GetTMSGlobalFileListRead() == false) {
            app.SetTMSAction(
                ProfileManager.GetPrimaryPad(),
                eTMSAction_TMSPP_RetrieveFiles_RunPlayGame
            );
        }
    }

#ifdef _XBOX
    if (!g_NetworkManager.IsHost()
        && !app.GetGameHostOption(eGameHostOption_FriendsOfFriends)) {
        if (m_userIndex == ProfileManager.GetPrimaryPad()) {
            for (DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
                if (ProfileManager.IsSignedIn(m_userIndex)
                    && ProfileManager.IsGuest(idx)) {
                    canPlay           = FALSE;
                    isFriendsWithHost = FALSE;
                } else {
                    PlayerUID playerXuid = INVALID_XUID;
                    if (ProfileManager.IsSignedInLive(idx)) {
                        ProfileManager.GetXUID(idx, &playerXuid, true);
                    }
                    if (playerXuid != INVALID_XUID) {
                        // Is this user friends with the host player?
                        BOOL  result;
                        DWORD error;
                        error = XUserAreUsersFriends(
                            idx,
                            &packet->m_playerXuids[packet->m_hostIndex],
                            1,
                            &result,
                            NULL
                        );
                        if (error == ERROR_SUCCESS && result != TRUE) {
                            canPlay           = FALSE;
                            isFriendsWithHost = FALSE;
                        }
                    }
                }
                if (!canPlay) break;
            }
        } else {
            if (ProfileManager.IsSignedIn(m_userIndex)
                && ProfileManager.IsGuest(m_userIndex)) {
                canPlay           = FALSE;
                isFriendsWithHost = FALSE;
            } else {
                PlayerUID playerXuid = INVALID_XUID;
                if (ProfileManager.IsSignedInLive(m_userIndex)) {
                    ProfileManager.GetXUID(m_userIndex, &playerXuid, true);
                }
                if (playerXuid != INVALID_XUID) {
                    // Is this user friends with the host player?
                    BOOL  result;
                    DWORD error;
                    error = XUserAreUsersFriends(
                        m_userIndex,
                        &packet->m_playerXuids[packet->m_hostIndex],
                        1,
                        &result,
                        NULL
                    );
                    if (error == ERROR_SUCCESS && result != TRUE) {
                        canPlay           = FALSE;
                        isFriendsWithHost = FALSE;
                    }
                }
            }
        }
    }

    if (canPlay) {
        for (DWORD i = 0; i < packet->m_dwPlayerCount; ++i) {
            bool localPlayer = false;
            for (DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
                if (ProfileManager.IsSignedInLive(idx)) {
                    // need to use the XUID here
                    PlayerUID playerXUID = INVALID_XUID;
                    if (!ProfileManager.IsGuest(idx)) {
                        // Guest don't have an offline XUID as they cannot play
                        // offline, so use their online one
                        ProfileManager.GetXUID(idx, &playerXUID, true);
                    }
                    if (ProfileManager.AreXUIDSEqual(
                            playerXUID,
                            packet->m_playerXuids[i]
                        ))
                        localPlayer = true;
                } else if (ProfileManager.IsSignedIn(idx)) {
                    // If we aren't signed into live then they have to be a
                    // local player
                    localPlayer = true;
                }
            }
            if (!localPlayer) {
                // First check our own permissions to see if we can play with
                // this player
                if (m_userIndex == ProfileManager.GetPrimaryPad()) {
                    canPlayLocal = ProfileManager.CanViewPlayerCreatedContent(
                        m_userIndex,
                        false,
                        &packet->m_playerXuids[i],
                        1
                    );

                    // 4J Stu - Everyone joining needs to have at least one
                    // friend in the game Local players are implied friends
                    if (isAtLeastOneFriend != TRUE) {
                        BOOL  result;
                        DWORD error;
                        for (DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
                            if (ProfileManager.IsSignedIn(idx)
                                && !ProfileManager.IsGuest(idx)) {
                                error = XUserAreUsersFriends(
                                    idx,
                                    &packet->m_playerXuids[i],
                                    1,
                                    &result,
                                    NULL
                                );
                                if (error == ERROR_SUCCESS && result == TRUE)
                                    isAtLeastOneFriend = TRUE;
                            }
                        }
                    }
                } else {
                    // Friends with the primary player on this system
                    isAtLeastOneFriend = true;

                    canPlayLocal = ProfileManager.CanViewPlayerCreatedContent(
                        m_userIndex,
                        true,
                        &packet->m_playerXuids[i],
                        1
                    );
                }

                // If we can play with them, then check if they can play with us
                if (canPlayLocal && (packet->m_friendsOnlyBits & (1 << i))) {
                    // If this is the primary pad then check all local players
                    // against the list in the packet (this happens when joining
                    // the game for the first time) If not the primary pad, then
                    // just check against this index. It happens on joining at
                    // the start, but more importantly players trying to join
                    // during the game
                    bool thisQuadrantOnly = true;
                    if (m_userIndex == ProfileManager.GetPrimaryPad())
                        thisQuadrantOnly = false;

                    BOOL  result;
                    DWORD error;
                    for (DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx) {
                        if ((!thisQuadrantOnly || m_userIndex == idx)
                            && ProfileManager.IsSignedIn(idx)
                            && !ProfileManager.IsGuest(idx)) {
                            error = XUserAreUsersFriends(
                                idx,
                                &packet->m_playerXuids[i],
                                1,
                                &result,
                                NULL
                            );
                            if (error == ERROR_SUCCESS) canPlay &= result;
                        }
                        if (!canPlay) break;
                    }
                }
                if (!canPlay || !canPlayLocal) break;
            }
        }
    }
#else
    // TODO - handle this kind of things for non-360 platforms
    canPlay                   = TRUE;
    canPlayLocal              = TRUE;
    isAtLeastOneFriend        = TRUE;
    cantPlayContentRestricted = FALSE;

#if (defined __PS3__ || defined __ORBIS__ || defined __PSVITA__)

    if (!g_NetworkManager.IsHost()
        && !app.GetGameHostOption(eGameHostOption_FriendsOfFriends)) {
        bool bChatRestricted = false;

        ProfileManager.GetChatAndContentRestrictions(
            m_userIndex,
            true,
            &bChatRestricted,
            NULL,
            NULL
        );

        // Chat restricted orbis players can still play online
#ifndef __ORBIS__
        canPlay = !bChatRestricted;
#endif

        if (m_userIndex == ProfileManager.GetPrimaryPad()) {
            // Is this user friends with the host player?
            bool         isFriend    = true;
            unsigned int friendCount = 0;
#ifdef __PS3__
            int ret = sceNpBasicGetFriendListEntryCount(&friendCount);
#elif defined __PSVITA__
            sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::FriendsList>
                friendList;
            int ret = -1;
            if (!CGameNetworkManager::usingAdhocMode()) // we don't need to be
                                                        // friends in PSN for
                                                        // adhoc mode
            {
                int ret = sce::Toolkit::NP::Friends::Interface::getFriendslist(
                    &friendList,
                    false
                );
                if (ret == SCE_TOOLKIT_NP_SUCCESS) {
                    if (friendList.hasResult()) {
                        friendCount = friendList.get()->size();
                    }
                }
            }
#else // __ORBIS__

            sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::FriendsList>
                friendList;

            sce::Toolkit::NP::FriendInfoRequest requestParam;
            memset(&requestParam, 0, sizeof(requestParam));
            requestParam.flag   = SCE_TOOLKIT_NP_FRIENDS_LIST_ALL;
            requestParam.limit  = 0;
            requestParam.offset = 0;
            requestParam.userInfo.userId =
                ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

            int ret = sce::Toolkit::NP::Friends::Interface::getFriendslist(
                &friendList,
                &requestParam,
                false
            );
            if (ret == 0) {
                if (friendList.hasResult()) {
                    friendCount = friendList.get()->size();
                }
            }
#endif
            if (ret == 0) {
                isFriend = false;
                SceNpId npid;
                for (unsigned int i = 0; i < friendCount; i++) {
#ifdef __PS3__
                    ret = sceNpBasicGetFriendListEntry(i, &npid);
#else
                    npid = friendList.get()->at(i).npid;
#endif
                    if (ret == 0) {
                        if (strcmp(
                                npid.handle.data,
                                packet->m_playerXuids[packet->m_hostIndex]
                                    .getOnlineID()
                            )
                            == 0) {
                            isFriend = true;
                            break;
                        }
                    }
                }
            }

            if (!isFriend) {
                canPlay           = FALSE;
                isFriendsWithHost = FALSE;
            }
        }
    }
    // is it an online game, and a player has chat restricted?
    else if (!g_NetworkManager.IsLocalGame()) {
        // if the player is chat restricted, then they can't play an online game
        bool bChatRestricted    = false;
        bool bContentRestricted = false;

        // If this is a pre-login packet for the first player on the machine,
        // then accumulate up these flags for everyone signed in. We can handle
        // exiting the game much more cleanly at this point by exiting the
        // level, rather than waiting for a prelogin packet for the other
        // players, when we have to exit the player which seems to be very
        // unstable at the point of starting up the game
        if (m_userIndex == ProfileManager.GetPrimaryPad()) {
            ProfileManager.GetChatAndContentRestrictions(
                m_userIndex,
                false,
                &bChatRestricted,
                &bContentRestricted,
                NULL
            );
        } else {
            ProfileManager.GetChatAndContentRestrictions(
                m_userIndex,
                true,
                &bChatRestricted,
                &bContentRestricted,
                NULL
            );
        }

        // Chat restricted orbis players can still play online
#ifndef __ORBIS__
        canPlayLocal = !bChatRestricted;
#endif

        cantPlayContentRestricted = bContentRestricted ? 1 : 0;
    }


#endif

#ifdef _XBOX_ONE
    if (!g_NetworkManager.IsHost()
        && m_userIndex == ProfileManager.GetPrimaryPad()) {
        long long startTime = System::currentTimeMillis();

        auto friendsXuids = DQRNetworkManager::GetFriends();

        if (app.GetGameHostOption(eGameHostOption_FriendsOfFriends)) {
            // Check that the user has at least one friend in the game
            isAtLeastOneFriend = false;

            for (int i = 0; i < friendsXuids->Size; i++) {
                auto friendsXuid = friendsXuids->GetAt(i);

                // Check this friend against each player, if we find them we
                // have at least one friend
                for (int j = 0; j < g_NetworkManager.GetPlayerCount(); j++) {
                    Platform::String ^ xboxUserId =
                        ref new Platform::String(g_NetworkManager
                                                     .GetPlayerByIndex(j)
                                                     ->GetUID()
                                                     .toString()
                                                     .data());
                    if (friendsXuid == xboxUserId) {
                        isAtLeastOneFriend = true;
                        break;
                    }
                }
            }

            app.DebugPrintf(
                "ClientConnection::handlePreLogin: User has at least one "
                "friend? %s\n",
                isAtLeastOneFriend ? "Yes" : "No"
            );
        } else {
            // Check that the user is friends with the host
            bool isFriend = false;

            Platform::String ^ hostXboxUserId = ref new Platform::String(
                g_NetworkManager.GetHostPlayer()->GetUID().toString().data()
            );

            for (int i = 0; i < friendsXuids->Size; i++) {
                if (friendsXuids->GetAt(i) == hostXboxUserId) {
                    isFriend = true;
                    break;
                }
            }

            if (!isFriend) {
                canPlay           = FALSE;
                isFriendsWithHost = FALSE;
            }

            app.DebugPrintf(
                "ClientConnection::handlePreLogin: User is friends with the "
                "host? %s\n",
                isFriendsWithHost ? "Yes" : "No"
            );
        }

        app.DebugPrintf(
            "ClientConnection::handlePreLogin: Friendship checks took %i ms\n",
            System::currentTimeMillis() - startTime
        );
    }
#endif

#endif // _XBOX

    if (!canPlay || !canPlayLocal || !isAtLeastOneFriend
        || cantPlayContentRestricted) {
#ifndef __PS3__
        DisconnectPacket::eDisconnectReason reason =
            DisconnectPacket::eDisconnect_NoUGC_Remote;
#else
        DisconnectPacket::eDisconnectReason reason =
            DisconnectPacket::eDisconnect_None;
#endif
        if (m_userIndex == ProfileManager.GetPrimaryPad()) {
            if (!isFriendsWithHost)
                reason = DisconnectPacket::eDisconnect_NotFriendsWithHost;
            else if (!isAtLeastOneFriend)
                reason = DisconnectPacket::eDisconnect_NoFriendsInGame;
            else if (!canPlayLocal)
                reason = DisconnectPacket::eDisconnect_NoUGC_AllLocal;
            else if (cantPlayContentRestricted)
                reason =
                    DisconnectPacket::eDisconnect_ContentRestricted_AllLocal;

            app.DebugPrintf(
                "Exiting world on handling Pre-Login packet due UGC "
                "privileges: %d\n",
                reason
            );
            app.SetDisconnectReason(reason);
            app.SetAction(
                ProfileManager.GetPrimaryPad(),
                eAppAction_ExitWorld,
                (void*)TRUE
            );
        } else {
            if (!isFriendsWithHost)
                reason = DisconnectPacket::eDisconnect_NotFriendsWithHost;
            else if (!canPlayLocal)
                reason = DisconnectPacket::eDisconnect_NoUGC_Single_Local;
            else if (cantPlayContentRestricted)
                reason = DisconnectPacket::
                    eDisconnect_ContentRestricted_Single_Local;

            app.DebugPrintf(
                "Exiting player %d on handling Pre-Login packet due UGC "
                "privileges: %d\n",
                m_userIndex,
                reason
            );
            UINT uiIDA[1];
            uiIDA[0] = IDS_CONFIRM_OK;
            if (!isFriendsWithHost)
                ui.RequestMessageBox(
                    IDS_CANTJOIN_TITLE,
                    IDS_NOTALLOWED_FRIENDSOFFRIENDS,
                    uiIDA,
                    1,
                    m_userIndex,
                    NULL,
                    NULL,
                    app.GetStringTable()
                );
            else
                ui.RequestMessageBox(
                    IDS_CANTJOIN_TITLE,
                    IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL,
                    uiIDA,
                    1,
                    m_userIndex,
                    NULL,
                    NULL,
                    app.GetStringTable()
                );

            app.SetDisconnectReason(reason);

            // 4J-PB - this locks up on the read and write threads not closing
            // down, because they are trying to lock the incoming critsec when
            // it's already locked by this thread
            // 			Minecraft::GetInstance()->connectionDisconnected(
            // m_userIndex , reason ); 			done = true;
            // connection->flush(); 			connection->close(reason);
            //			app.SetAction(m_userIndex,eAppAction_ExitPlayer);

            // 4J-PB - doing this instead
            app.SetAction(m_userIndex, eAppAction_ExitPlayerPreLogin);
        }
    } else {
        // Texture pack handling
        // If we have the texture pack for the game, load it
        // If we don't then send a packet to the host to request it. We need to
        // send this before the LoginPacket so that it gets handled first, as
        // once the LoginPacket is received on the client the game is close to
        // starting
        if (m_userIndex == ProfileManager.GetPrimaryPad()) {
            Minecraft* pMinecraft = Minecraft::GetInstance();
            if (pMinecraft->skins->selectTexturePackById(
                    packet->m_texturePackId
                )) {
                app.DebugPrintf(
                    "Selected texture pack %d from Pre-Login packet\n",
                    packet->m_texturePackId
                );
            } else {
                app.DebugPrintf(
                    "Could not select texture pack %d from Pre-Login packet, "
                    "requesting from host\n",
                    packet->m_texturePackId
                );

                // 4J-PB - we need to upsell the texture pack to the player
                // app.SetAction(m_userIndex,eAppAction_TexturePackRequired);
                // Let the player go into the game, and we'll check that they
                // are using the right texture pack when in
            }
        }

        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCPreLoginReceived * 100) / (eCCConnected)
            );
        }
        // need to use the XUID here
        PlayerUID offlineXUID = INVALID_XUID;
        PlayerUID onlineXUID  = INVALID_XUID;
        if (ProfileManager.IsSignedInLive(m_userIndex)) {
            // Guest don't have an offline XUID as they cannot play offline, so
            // use their online one
            ProfileManager.GetXUID(m_userIndex, &onlineXUID, true);
        }
#ifdef __PSVITA__
        if (CGameNetworkManager::usingAdhocMode()
            && onlineXUID.getOnlineID()[0] == 0) {
            // player doesn't have an online UID, set it from the player name
            onlineXUID.setForAdhoc();
        }
#endif

        // On PS3, all non-signed in players (even guests) can get a useful
        // offlineXUID
#if !(defined __PS3__ || defined _DURANGO)
        if (!ProfileManager.IsGuest(m_userIndex))
#endif
        {
            // All other players we use their offline XUID so that they can play
            // the game offline
            ProfileManager.GetXUID(m_userIndex, &offlineXUID, false);
        }
        BOOL allAllowed, friendsAllowed;
        ProfileManager.AllowedPlayerCreatedContent(
            m_userIndex,
            true,
            &allAllowed,
            &friendsAllowed
        );
        send(
            std::shared_ptr<LoginPacket>(new LoginPacket(
                minecraft->user->name,
                SharedConstants::NETWORK_PROTOCOL_VERSION,
                offlineXUID,
                onlineXUID,
                (allAllowed != TRUE && friendsAllowed == TRUE),
                packet->m_ugcPlayersVersion,
                app.GetPlayerSkinId(m_userIndex),
                app.GetPlayerCapeId(m_userIndex),
                ProfileManager.IsGuest(m_userIndex)
            ))
        );

        if (!g_NetworkManager.IsHost()) {
            Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
                (eCCLoginSent * 100) / (eCCConnected)
            );
        }
    }
#else
    // 4J - removed
    if (packet->loginKey.equals("-")) {
        send(new LoginPacket(
            minecraft->user.name,
            SharedConstants.NETWORK_PROTOCOL_VERSION
        ));
    } else {
        try {
            URL url = new URL(
                "http://www.minecraft->net/game/joinserver.jsp?user="
                + minecraft->user.name + "&sessionId="
                + minecraft->user.sessionId + "&serverId=" + packet->loginKey
            );
            BufferedReader br =
                new BufferedReader(new InputStreamReader(url.openStream()));
            String msg = br.readLine();
            br.close();

            if (msg.equalsIgnoreCase("ok")) {
                send(new LoginPacket(
                    minecraft->user.name,
                    SharedConstants.NETWORK_PROTOCOL_VERSION
                ));
            } else {
                connection.close("disconnect.loginFailedInfo", msg);
            }
        } catch (Exception e) {
            e.printStackTrace();
            connection.close(
                "disconnect.genericReason",
                "Internal client error: " + e.toString()
            );
        }
    }
#endif
}

void ClientConnection::close() {
    // If it's already done, then we don't need to do anything here. And in fact
    // trying to do something could cause a crash
    if (done) return;
    done = true;
    connection->flush();
    connection->close(DisconnectPacket::eDisconnect_Closed);
}

void ClientConnection::handleAddMob(std::shared_ptr<AddMobPacket> packet) {
    double x    = packet->x / 32.0;
    double y    = packet->y / 32.0;
    double z    = packet->z / 32.0;
    float  yRot = packet->yRot * 360 / 256.0f;
    float  xRot = packet->xRot * 360 / 256.0f;

    std::shared_ptr<Mob> mob =
        std::dynamic_pointer_cast<Mob>(EntityIO::newById(packet->type, level));
    mob->xp       = packet->x;
    mob->yp       = packet->y;
    mob->zp       = packet->z;
    mob->yHeadRot = packet->yHeadRot * 360 / 256.0f;
    mob->yRotp    = packet->yRot;
    mob->xRotp    = packet->xRot;

    std::vector<std::shared_ptr<Entity>>* subEntities = mob->getSubEntities();
    if (subEntities != NULL) {
        int offs = packet->id - mob->entityId;
        // for (int i = 0; i < subEntities.length; i++)
        for (auto it = subEntities->begin(); it != subEntities->end(); ++it) {
            // subEntities[i].entityId += offs;
            (*it)->entityId += offs;
        }
    }

    mob->entityId = packet->id;

    //	printf("\t\t\t\t%d: Add mob rot %d\n",packet->id,packet->yRot);

    mob->absMoveTo(x, y, z, yRot, xRot);
    mob->xd = packet->xd / 8000.0f;
    mob->yd = packet->yd / 8000.0f;
    mob->zd = packet->zd / 8000.0f;
    level->putEntity(packet->id, mob);

    std::vector<std::shared_ptr<SynchedEntityData::DataItem>>* unpackedData =
        packet->getUnpackedData();
    if (unpackedData != NULL) {
        mob->getEntityData()->assignValues(unpackedData);
    }

    // Fix for #65236 - TU8: Content: Gameplay: Magma Cubes' have strange hit
    // boxes. 4J Stu - Slimes have a different BB depending on their size which
    // is set in the entity data, so update the BB
    if (mob->GetType() == eTYPE_SLIME || mob->GetType() == eTYPE_LAVASLIME) {
        std::shared_ptr<Slime> slime = std::dynamic_pointer_cast<Slime>(mob);
        slime->setSize(slime->getSize());
    }
}

void ClientConnection::handleSetTime(std::shared_ptr<SetTimePacket> packet) {
    minecraft->level->setTime(packet->time);
}

void ClientConnection::handleSetSpawn(
    std::shared_ptr<SetSpawnPositionPacket> packet
) {
    // minecraft->player->setRespawnPosition(new Pos(packet->x, packet->y,
    // packet->z));
    minecraft->localplayers[m_userIndex]->setRespawnPosition(
        new Pos(packet->x, packet->y, packet->z)
    );
    minecraft->level->getLevelData()->setSpawn(packet->x, packet->y, packet->z);
}

void ClientConnection::handleRidePacket(
    std::shared_ptr<SetRidingPacket> packet
) {
    std::shared_ptr<Entity> rider  = getEntity(packet->riderId);
    std::shared_ptr<Entity> ridden = getEntity(packet->riddenId);

    std::shared_ptr<Boat> boat = std::dynamic_pointer_cast<Boat>(ridden);
    // if (packet->riderId == minecraft->player->entityId) rider =
    // minecraft->player;
    if (packet->riderId == minecraft->localplayers[m_userIndex]->entityId) {
        rider = minecraft->localplayers[m_userIndex];

        if (boat) boat->setDoLerp(false);
    } else if (boat) {
        boat->setDoLerp(true);
    }
    if (rider == NULL) return;

    rider->ride(ridden);
}

void ClientConnection::handleEntityEvent(
    std::shared_ptr<EntityEventPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->entityId);
    if (e != NULL) e->handleEntityEvent(packet->eventId);
}

std::shared_ptr<Entity> ClientConnection::getEntity(int entityId) {
    // if (entityId == minecraft->player->entityId)
    if (entityId == minecraft->localplayers[m_userIndex]->entityId) {
        // return minecraft->player;
        return minecraft->localplayers[m_userIndex];
    }
    return level->getEntity(entityId);
}

void ClientConnection::handleSetHealth(
    std::shared_ptr<SetHealthPacket> packet
) {
    // minecraft->player->hurtTo(packet->health);
    minecraft->localplayers[m_userIndex]->hurtTo(
        packet->health,
        packet->damageSource
    );
    minecraft->localplayers[m_userIndex]->getFoodData()->setFoodLevel(
        packet->food
    );
    minecraft->localplayers[m_userIndex]->getFoodData()->setSaturation(
        packet->saturation
    );

    // We need food
    if (packet->food < FoodConstants::HEAL_LEVEL - 1) {
        if (minecraft->localgameModes[m_userIndex] != NULL
            && !minecraft->localgameModes[m_userIndex]->hasInfiniteItems()) {
            minecraft->localgameModes[m_userIndex]
                ->getTutorial()
                ->changeTutorialState(e_Tutorial_State_Food_Bar);
        }
    }
}

void ClientConnection::handleSetExperience(
    std::shared_ptr<SetExperiencePacket> packet
) {
    minecraft->localplayers[m_userIndex]->setExperienceValues(
        packet->experienceProgress,
        packet->totalExperience,
        packet->experienceLevel
    );
}

void ClientConnection::handleTexture(std::shared_ptr<TexturePacket> packet) {
    // Both PlayerConnection and ClientConnection should handle this mostly the
    // same way Server side also needs to store a list of those clients waiting
    // to get a texture the server doesn't have yet so that it can send it out
    // to them when it comes in

    if (packet->dwBytes == 0) {
        // Request for texture
#ifndef _CONTENT_PACKAGE
        wprintf(
            L"Client received request for custom texture %ls\n",
            packet->textureName.c_str()
        );
#endif
        PBYTE pbData  = NULL;
        DWORD dwBytes = 0;
        app.GetMemFileDetails(packet->textureName, &pbData, &dwBytes);

        if (dwBytes != 0) {
            send(
                std::shared_ptr<TexturePacket>(
                    new TexturePacket(packet->textureName, pbData, dwBytes)
                )
            );
        }
    } else {
        // Response with texture data
#ifndef _CONTENT_PACKAGE
        wprintf(
            L"Client received custom texture %ls\n",
            packet->textureName.c_str()
        );
#endif
        app.AddMemoryTextureFile(
            packet->textureName,
            packet->pbData,
            packet->dwBytes
        );
        Minecraft::GetInstance()->handleClientTextureReceived(
            packet->textureName
        );
    }
}

void ClientConnection::handleTextureAndGeometry(
    std::shared_ptr<TextureAndGeometryPacket> packet
) {
    // Both PlayerConnection and ClientConnection should handle this mostly the
    // same way Server side also needs to store a list of those clients waiting
    // to get a texture the server doesn't have yet so that it can send it out
    // to them when it comes in

    if (packet->dwTextureBytes == 0) {
        // Request for texture
#ifndef _CONTENT_PACKAGE
        wprintf(
            L"Client received request for custom texture and geometry %ls\n",
            packet->textureName.c_str()
        );
#endif
        PBYTE pbData  = NULL;
        DWORD dwBytes = 0;
        app.GetMemFileDetails(packet->textureName, &pbData, &dwBytes);
        DLCSkinFile* pDLCSkinFile =
            app.m_dlcManager.getSkinFile(packet->textureName);

        if (dwBytes != 0) {
            if (pDLCSkinFile) {
                if (pDLCSkinFile->getAdditionalBoxesCount() != 0) {
                    send(
                        std::shared_ptr<TextureAndGeometryPacket>(
                            new TextureAndGeometryPacket(
                                packet->textureName,
                                pbData,
                                dwBytes,
                                pDLCSkinFile
                            )
                        )
                    );
                } else {
                    send(
                        std::shared_ptr<TextureAndGeometryPacket>(
                            new TextureAndGeometryPacket(
                                packet->textureName,
                                pbData,
                                dwBytes
                            )
                        )
                    );
                }
            } else {
                unsigned int uiAnimOverrideBitmask =
                    app.GetAnimOverrideBitmask(packet->dwSkinID);

                send(
                    std::shared_ptr<TextureAndGeometryPacket>(
                        new TextureAndGeometryPacket(
                            packet->textureName,
                            pbData,
                            dwBytes,
                            app.GetAdditionalSkinBoxes(packet->dwSkinID),
                            uiAnimOverrideBitmask
                        )
                    )
                );
            }
        }
    } else {
        // Response with texture data
#ifndef _CONTENT_PACKAGE
        wprintf(
            L"Client received custom TextureAndGeometry %ls\n",
            packet->textureName.c_str()
        );
#endif
        // Add the texture data
        app.AddMemoryTextureFile(
            packet->textureName,
            packet->pbData,
            packet->dwTextureBytes
        );
        // Add the geometry data
        if (packet->dwBoxC != 0) {
            app.SetAdditionalSkinBoxes(
                packet->dwSkinID,
                packet->BoxDataA,
                packet->dwBoxC
            );
        }
        // Add the anim override
        app.SetAnimOverrideBitmask(
            packet->dwSkinID,
            packet->uiAnimOverrideBitmask
        );

        // clear out the pending texture request
        Minecraft::GetInstance()->handleClientTextureReceived(
            packet->textureName
        );
    }
}

void ClientConnection::handleTextureChange(
    std::shared_ptr<TextureChangePacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e);
    if (e == NULL) return;

    bool isLocalPlayer = false;
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (minecraft->localplayers[i]) {
            if (minecraft->localplayers[i]->entityId == packet->id) {
                isLocalPlayer = true;
                break;
            }
        }
    }
    if (isLocalPlayer) return;

    switch (packet->action) {
    case TextureChangePacket::e_TextureChange_Skin:
        player->setCustomSkin(app.getSkinIdFromPath(packet->path));
#ifndef _CONTENT_PACKAGE
        wprintf(
            L"Skin for remote player %ls has changed to %ls (%d)\n",
            player->name.c_str(),
            player->customTextureUrl.c_str(),
            player->getPlayerDefaultSkin()
        );
#endif
        break;
    case TextureChangePacket::e_TextureChange_Cape:
        player->setCustomCape(Player::getCapeIdFromPath(packet->path));
        // player->customTextureUrl2 = packet->path;
#ifndef _CONTENT_PACKAGE
        wprintf(
            L"Cape for remote player %ls has changed to %ls\n",
            player->name.c_str(),
            player->customTextureUrl2.c_str()
        );
#endif
        break;
    }

    if (!packet->path.empty() && packet->path.substr(0, 3).compare(L"def") != 0
        && !app.IsFileInMemoryTextures(packet->path)) {
        if (minecraft->addPendingClientTextureRequest(packet->path)) {
#ifndef _CONTENT_PACKAGE
            wprintf(
                L"handleTextureChange - Client sending texture packet to get "
                L"custom skin %ls for player %ls\n",
                packet->path.c_str(),
                player->name.c_str()
            );
#endif
            send(
                std::shared_ptr<TexturePacket>(
                    new TexturePacket(packet->path, NULL, 0)
                )
            );
        }
    } else if (!packet->path.empty()
               && app.IsFileInMemoryTextures(packet->path)) {
        // Update the ref count on the memory texture data
        app.AddMemoryTextureFile(packet->path, NULL, 0);
    }
}

void ClientConnection::handleTextureAndGeometryChange(
    std::shared_ptr<TextureAndGeometryChangePacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->id);
    if (e == NULL) return;
    std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(e);
    if (e == NULL) return;

    bool isLocalPlayer = false;
    for (int i = 0; i < XUSER_MAX_COUNT; i++) {
        if (minecraft->localplayers[i]) {
            if (minecraft->localplayers[i]->entityId == packet->id) {
                isLocalPlayer = true;
                break;
            }
        }
    }
    if (isLocalPlayer) return;


    player->setCustomSkin(app.getSkinIdFromPath(packet->path));

#ifndef _CONTENT_PACKAGE
    wprintf(
        L"Skin for remote player %ls has changed to %ls (%d)\n",
        player->name.c_str(),
        player->customTextureUrl.c_str(),
        player->getPlayerDefaultSkin()
    );
#endif

    if (!packet->path.empty() && packet->path.substr(0, 3).compare(L"def") != 0
        && !app.IsFileInMemoryTextures(packet->path)) {
        if (minecraft->addPendingClientTextureRequest(packet->path)) {
#ifndef _CONTENT_PACKAGE
            wprintf(
                L"handleTextureAndGeometryChange - Client sending "
                L"TextureAndGeometryPacket to get custom skin %ls for player "
                L"%ls\n",
                packet->path.c_str(),
                player->name.c_str()
            );
#endif
            send(
                std::shared_ptr<TextureAndGeometryPacket>(
                    new TextureAndGeometryPacket(packet->path, NULL, 0)
                )
            );
        }
    } else if (!packet->path.empty()
               && app.IsFileInMemoryTextures(packet->path)) {
        // Update the ref count on the memory texture data
        app.AddMemoryTextureFile(packet->path, NULL, 0);
    }
}

void ClientConnection::handleRespawn(std::shared_ptr<RespawnPacket> packet) {
    // if (packet->dimension != minecraft->player->dimension)
    if (packet->dimension != minecraft->localplayers[m_userIndex]->dimension
        || packet->mapSeed
               != minecraft->localplayers[m_userIndex]->level->getSeed()) {
        int oldDimension = minecraft->localplayers[m_userIndex]->dimension;
        started          = false;

        // Remove client connection from this level
        level->removeClientConnection(this, false);

        MultiPlayerLevel* dimensionLevel =
            (MultiPlayerLevel*)minecraft->getLevel(packet->dimension);
        if (dimensionLevel == NULL) {
            dimensionLevel = new MultiPlayerLevel(
                this,
                new LevelSettings(
                    packet->mapSeed,
                    packet->playerGameType,
                    false,
                    minecraft->level->getLevelData()->isHardcore(),
                    packet->m_newSeaLevel,
                    packet->m_pLevelType,
                    packet->m_xzSize,
                    packet->m_hellScale
                ),
                packet->dimension,
                packet->difficulty
            );

            // 4J Stu - We want to shared the savedDataStorage between both
            // levels
            // if( dimensionLevel->savedDataStorage != NULL )
            //{
            // Don't need to delete it here as it belongs to a client connection
            // while will delete it when it's done
            //	delete dimensionLevel->savedDataStorage;+
            //}
            dimensionLevel->savedDataStorage = level->savedDataStorage;

            dimensionLevel->difficulty = packet->difficulty; // 4J Added
            app.DebugPrintf(
                "dimensionLevel->difficulty - Difficulty = %d\n",
                packet->difficulty
            );

            dimensionLevel->isClientSide = true;
        } else {
            dimensionLevel->addClientConnection(this);
        }

        // Remove the player entity from the current level
        level->removeEntity(
            std::shared_ptr<Entity>(minecraft->localplayers[m_userIndex])
        );

        level = dimensionLevel;

        // Whilst calling setLevel, make sure that minecraft::player is set up
        // to be correct for this connection
        std::shared_ptr<MultiplayerLocalPlayer> lastPlayer = minecraft->player;
        minecraft->player = minecraft->localplayers[m_userIndex];
        minecraft->setLevel(dimensionLevel);
        minecraft->player = lastPlayer;

        TelemetryManager->RecordLevelExit(
            m_userIndex,
            eSen_LevelExitStatus_Succeeded
        );

        // minecraft->player->dimension = packet->dimension;
        minecraft->localplayers[m_userIndex]->dimension = packet->dimension;
        // minecraft->setScreen(new ReceivingLevelScreen(this));
        //		minecraft->addPendingLocalConnection(m_userIndex, this);

#ifdef _XBOX
        TelemetryManager->RecordLevelStart(
            m_userIndex,
            eSen_FriendOrMatch_Playing_With_Invited_Friends,
            eSen_CompeteOrCoop_Coop_and_Competitive,
            Minecraft::GetInstance()->getLevel(packet->dimension)->difficulty,
            app.GetLocalPlayerCount(),
            g_NetworkManager.GetOnlinePlayerCount()
        );
#endif

        if (minecraft->localgameModes[m_userIndex] != NULL) {
            TutorialMode* gameMode =
                (TutorialMode*)minecraft->localgameModes[m_userIndex];
            gameMode->getTutorial()->showTutorialPopup(false);
        }

        // 4J-JEV: Fix for Durango #156334 - Content: UI: Rich Presence 'In the
        // Nether' message is updating with a 3 to 10 minute  delay.
        minecraft->localplayers[m_userIndex]->updateRichPresence();

        ConnectionProgressParams* param = new ConnectionProgressParams();
        param->iPad                     = m_userIndex;
        if (packet->dimension == -1) {
            param->stringId = IDS_PROGRESS_ENTERING_NETHER;
        } else if (oldDimension == -1) {
            param->stringId = IDS_PROGRESS_LEAVING_NETHER;
        } else if (packet->dimension == 1) {
            param->stringId = IDS_PROGRESS_ENTERING_END;
        } else if (oldDimension == 1) {
            param->stringId = IDS_PROGRESS_LEAVING_END;
        }
        param->showTooltips = false;
        param->setFailTimer = false;

        // 4J Stu - Fix for #13543 - Crash: Game crashes if entering a portal
        // with the inventory menu open
        ui.CloseUIScenes(m_userIndex);

        if (app.GetLocalPlayerCount() > 1) {
            ui.NavigateToScene(m_userIndex, eUIScene_ConnectingProgress, param);
        } else {
            ui.NavigateToScene(m_userIndex, eUIScene_ConnectingProgress, param);
        }

        app.SetAction(m_userIndex, eAppAction_WaitForDimensionChangeComplete);
    }

    // minecraft->respawnPlayer(minecraft->player->GetXboxPad(),true,
    // packet->dimension);

    // Wrap respawnPlayer call up in code to set & restore the player/gamemode
    // etc. as some things in there assume that we are set up for the player
    // that the respawn is coming in for
    int oldIndex = minecraft->getLocalPlayerIdx();
    minecraft->setLocalPlayerIdx(m_userIndex);
    minecraft->respawnPlayer(
        minecraft->localplayers[m_userIndex]->GetXboxPad(),
        packet->dimension,
        packet->m_newEntityId
    );
    ((MultiPlayerGameMode*)minecraft->localgameModes[m_userIndex])
        ->setLocalMode(packet->playerGameType);
    minecraft->setLocalPlayerIdx(oldIndex);
}

void ClientConnection::handleExplosion(std::shared_ptr<ExplodePacket> packet) {
    if (!packet->m_bKnockbackOnly) {
        // app.DebugPrintf("Received ExplodePacket with explosion data\n");
        PIXBeginNamedEvent(0, "Handling explosion");
        Explosion* e = new Explosion(
            minecraft->level,
            nullptr,
            packet->x,
            packet->y,
            packet->z,
            packet->r
        );
        PIXBeginNamedEvent(0, "Finalizing");

        // Fix for #81758 - TCR 006 BAS Non-Interactive Pause: TU9: Performance:
        // Gameplay: After detonating bunch of TNT, game enters unresponsive
        // state for couple of seconds. The changes we are making here have been
        // decided by the server, so we don't need to add them to the vector
        // that resets tiles changes made on the client as we KNOW that the
        // server is matching these changes
        MultiPlayerLevel* mpLevel = (MultiPlayerLevel*)minecraft->level;
        mpLevel->enableResetChanges(false);
        // 4J - now directly pass a pointer to the toBlow array in the packet
        // rather than copying around
        e->finalizeExplosion(true, &packet->toBlow);
        mpLevel->enableResetChanges(true);
        PIXEndNamedEvent();
        PIXEndNamedEvent();
        delete e;
    } else {
        // app.DebugPrintf("Received ExplodePacket with knockback only data\n");
    }

    // app.DebugPrintf("Adding knockback (%f,%f,%f) for player %d\n",
    // packet->getKnockbackX(), packet->getKnockbackY(),
    // packet->getKnockbackZ(), m_userIndex);
    minecraft->localplayers[m_userIndex]->xd += packet->getKnockbackX();
    minecraft->localplayers[m_userIndex]->yd += packet->getKnockbackY();
    minecraft->localplayers[m_userIndex]->zd += packet->getKnockbackZ();
}

void ClientConnection::handleContainerOpen(
    std::shared_ptr<ContainerOpenPacket> packet
) {
    bool                                    failed = false;
    std::shared_ptr<MultiplayerLocalPlayer> player =
        minecraft->localplayers[m_userIndex];
    switch (packet->type) {
    case ContainerOpenPacket::CONTAINER: {
        if (player->openContainer(
                std::shared_ptr<SimpleContainer>(
                    new SimpleContainer(packet->title, packet->size)
                )
            )) {
            player->containerMenu->containerId = packet->containerId;
        } else {
            failed = true;
        }
    } break;
    case ContainerOpenPacket::FURNACE: {
        if (player->openFurnace(
                std::shared_ptr<FurnaceTileEntity>(new FurnaceTileEntity())
            )) {
            player->containerMenu->containerId = packet->containerId;
        } else {
            failed = true;
        }
    } break;
    case ContainerOpenPacket::BREWING_STAND: {
        if (player->openBrewingStand(
                std::shared_ptr<BrewingStandTileEntity>(
                    new BrewingStandTileEntity()
                )
            )) {
            player->containerMenu->containerId = packet->containerId;
        } else {
            failed = true;
        }
    } break;
    case ContainerOpenPacket::TRAP: {
        if (player->openTrap(
                std::shared_ptr<DispenserTileEntity>(new DispenserTileEntity())
            )) {
            player->containerMenu->containerId = packet->containerId;
        } else {
            failed = true;
        }
    } break;
    case ContainerOpenPacket::WORKBENCH: {
        if (player->startCrafting(
                Mth::floor(player->x),
                Mth::floor(player->y),
                Mth::floor(player->z)
            )) {
            player->containerMenu->containerId = packet->containerId;
        } else {
            failed = true;
        }
    } break;
    case ContainerOpenPacket::ENCHANTMENT: {
        if (player->startEnchanting(
                Mth::floor(player->x),
                Mth::floor(player->y),
                Mth::floor(player->z)
            )) {
            player->containerMenu->containerId = packet->containerId;
        } else {
            failed = true;
        }
    } break;
    case ContainerOpenPacket::TRADER_NPC: {
        std::shared_ptr<ClientSideMerchant> csm =
            std::shared_ptr<ClientSideMerchant>(
                new ClientSideMerchant(player, packet->title)
            );
        csm->createContainer();
        if (player->openTrading(csm)) {
            player->containerMenu->containerId = packet->containerId;
        } else {
            failed = true;
        }
    } break;
    case ContainerOpenPacket::REPAIR_TABLE: {
        if (player->startRepairing(
                Mth::floor(player->x),
                Mth::floor(player->y),
                Mth::floor(player->z)
            )) {
            player->containerMenu->containerId = packet->containerId;
        } else {
            failed = true;
        }
    } break;
    };

    if (failed) {
        // Failed - if we've got a non-inventory container currently here, close
        // that, which locally should put us back to not having a container
        // open, and should send a containerclose to the server so it doesn't
        // have a container open. If we don't have a non-inventory container
        // open, just send the packet, and again we ought to be in sync with the
        // server.
        if (player->containerMenu != player->inventoryMenu) {
            ui.CloseUIScenes(m_userIndex);
        } else {
            send(
                std::shared_ptr<ContainerClosePacket>(
                    new ContainerClosePacket(packet->containerId)
                )
            );
        }
    }
}

void ClientConnection::handleContainerSetSlot(
    std::shared_ptr<ContainerSetSlotPacket> packet
) {
    std::shared_ptr<MultiplayerLocalPlayer> player =
        minecraft->localplayers[m_userIndex];
    if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_CARRIED) {
        player->inventory->setCarried(packet->item);
    } else {
        if (packet->containerId
            == AbstractContainerMenu::CONTAINER_ID_INVENTORY) {
            // 4J Stu - Reworked a bit to fix a bug where things being collected
            // while the creative menu was up replaced items in the creative
            // menu
            if (packet->slot >= 36 && packet->slot < 36 + 9) {
                std::shared_ptr<ItemInstance> lastItem =
                    player->inventoryMenu->getSlot(packet->slot)->getItem();
                if (packet->item != NULL) {
                    if (lastItem == NULL
                        || lastItem->count < packet->item->count) {
                        packet->item->popTime = Inventory::POP_TIME_DURATION;
                    }
                }
            }
            player->inventoryMenu->setItem(packet->slot, packet->item);
        } else if (packet->containerId == player->containerMenu->containerId) {
            player->containerMenu->setItem(packet->slot, packet->item);
        }
    }
}

void ClientConnection::handleContainerAck(
    std::shared_ptr<ContainerAckPacket> packet
) {
    std::shared_ptr<MultiplayerLocalPlayer> player =
        minecraft->localplayers[m_userIndex];
    AbstractContainerMenu* menu = NULL;
    if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_INVENTORY) {
        menu = player->inventoryMenu;
    } else if (packet->containerId == player->containerMenu->containerId) {
        menu = player->containerMenu;
    }
    if (menu != NULL) {
        if (!packet->accepted) {
            send(
                std::shared_ptr<ContainerAckPacket>(new ContainerAckPacket(
                    packet->containerId,
                    packet->uid,
                    true
                ))
            );
        }
    }
}

void ClientConnection::handleContainerContent(
    std::shared_ptr<ContainerSetContentPacket> packet
) {
    std::shared_ptr<MultiplayerLocalPlayer> player =
        minecraft->localplayers[m_userIndex];
    if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_INVENTORY) {
        player->inventoryMenu->setAll(&packet->items);
    } else if (packet->containerId == player->containerMenu->containerId) {
        player->containerMenu->setAll(&packet->items);
    }
}

void ClientConnection::handleSignUpdate(
    std::shared_ptr<SignUpdatePacket> packet
) {
    app.DebugPrintf("ClientConnection::handleSignUpdate - ");
    if (minecraft->level->hasChunkAt(packet->x, packet->y, packet->z)) {
        std::shared_ptr<TileEntity> te =
            minecraft->level->getTileEntity(packet->x, packet->y, packet->z);

        // 4J-PB - on a client connecting, the line below fails
        if (std::dynamic_pointer_cast<SignTileEntity>(te) != NULL) {
            std::shared_ptr<SignTileEntity> ste =
                std::dynamic_pointer_cast<SignTileEntity>(te);
            for (int i = 0; i < MAX_SIGN_LINES; i++) {
                ste->SetMessage(i, packet->lines[i]);
            }

            app.DebugPrintf(
                "verified = %d\tCensored = %d\n",
                packet->m_bVerified,
                packet->m_bCensored
            );
            ste->SetVerified(packet->m_bVerified);
            ste->SetCensored(packet->m_bCensored);

            ste->setChanged();
        } else {
            app.DebugPrintf(
                "std::dynamic_pointer_cast<SignTileEntity>(te) == NULL\n"
            );
        }
    } else {
        app.DebugPrintf("hasChunkAt failed\n");
    }
}

void ClientConnection::handleTileEntityData(
    std::shared_ptr<TileEntityDataPacket> packet
) {
    if (minecraft->level->hasChunkAt(packet->x, packet->y, packet->z)) {
        std::shared_ptr<TileEntity> te =
            minecraft->level->getTileEntity(packet->x, packet->y, packet->z);

        if (te != NULL) {
            if (packet->type == TileEntityDataPacket::TYPE_MOB_SPAWNER
                && std::dynamic_pointer_cast<MobSpawnerTileEntity>(te)
                       != NULL) {
                std::dynamic_pointer_cast<MobSpawnerTileEntity>(te)->load(
                    packet->tag
                );
            }
            // else if (packet.type == TileEntityDataPacket.TYPE_ADV_COMMAND &&
            // (te instanceof CommandBlockEntity))
            //{
            //	((CommandBlockEntity) te).load(packet.tag);
            // }
            // else if (packet.type == TileEntityDataPacket.TYPE_BEACON && (te
            // instanceof BeaconTileEntity))
            //{
            //	((BeaconTileEntity) te).load(packet.tag);
            // }
            else if (packet->type == TileEntityDataPacket::TYPE_SKULL
                     && std::dynamic_pointer_cast<SkullTileEntity>(te)
                            != NULL) {
                std::dynamic_pointer_cast<SkullTileEntity>(te)->load(
                    packet->tag
                );
            }
        }
    }
}

void ClientConnection::handleContainerSetData(
    std::shared_ptr<ContainerSetDataPacket> packet
) {
    onUnhandledPacket(packet);
    if (minecraft->localplayers[m_userIndex]->containerMenu != NULL
        && minecraft->localplayers[m_userIndex]->containerMenu->containerId
               == packet->containerId) {
        minecraft->localplayers[m_userIndex]->containerMenu->setData(
            packet->id,
            packet->value
        );
    }
}

void ClientConnection::handleSetEquippedItem(
    std::shared_ptr<SetEquippedItemPacket> packet
) {
    std::shared_ptr<Entity> entity = getEntity(packet->entity);
    if (entity != NULL) {
        // 4J Stu - Brought forward change from 1.3 to fix #64688 - Customer
        // Encountered: TU7: Content: Art: Aura of enchanted item is not
        // displayed for other players in online game
        entity->setEquippedSlot(packet->slot, packet->getItem());
    }
}

void ClientConnection::handleContainerClose(
    std::shared_ptr<ContainerClosePacket> packet
) {
    minecraft->localplayers[m_userIndex]->closeContainer();
}

void ClientConnection::handleTileEvent(
    std::shared_ptr<TileEventPacket> packet
) {
    PIXBeginNamedEvent(0, "Handle tile event\n");
    minecraft->level->tileEvent(
        packet->x,
        packet->y,
        packet->z,
        packet->tile,
        packet->b0,
        packet->b1
    );
    PIXEndNamedEvent();
}

void ClientConnection::handleTileDestruction(
    std::shared_ptr<TileDestructionPacket> packet
) {
    minecraft->level->destroyTileProgress(
        packet->getEntityId(),
        packet->getX(),
        packet->getY(),
        packet->getZ(),
        packet->getState()
    );
}

bool ClientConnection::canHandleAsyncPackets() {
    return minecraft != NULL && minecraft->level != NULL
        && minecraft->localplayers[m_userIndex] != NULL && level != NULL;
}

void ClientConnection::handleGameEvent(
    std::shared_ptr<GameEventPacket> gameEventPacket
) {
    int event = gameEventPacket->_event;
    int param = gameEventPacket->param;
    if (event >= 0 && event < GameEventPacket::EVENT_LANGUAGE_ID_LENGTH) {
        if (GameEventPacket::EVENT_LANGUAGE_ID[event]
            > 0) // 4J - was NULL check
        {
            minecraft->localplayers[m_userIndex]->displayClientMessage(
                GameEventPacket::EVENT_LANGUAGE_ID[event]
            );
        }
    }
    if (event == GameEventPacket::START_RAINING) {
        level->getLevelData()->setRaining(true);
        level->setRainLevel(1);
    } else if (event == GameEventPacket::STOP_RAINING) {
        level->getLevelData()->setRaining(false);
        level->setRainLevel(0);
    } else if (event == GameEventPacket::CHANGE_GAME_MODE) {
        minecraft->localgameModes[m_userIndex]->setLocalMode(
            GameType::byId(param)
        );
    } else if (event == GameEventPacket::WIN_GAME) {
        ui.SetWinUserIndex((BYTE)gameEventPacket->param);

#ifdef _XBOX

        // turn off the gamertags in splitscreen for the primary player, since
        // they are about to be made fullscreen
        ui.HideAllGameUIElements();

        // Hide the other players scenes
        ui.ShowOtherPlayersBaseScene(ProfileManager.GetPrimaryPad(), false);

        // This just allows it to be shown
        if (minecraft->localgameModes[ProfileManager.GetPrimaryPad()] != NULL)
            minecraft->localgameModes[ProfileManager.GetPrimaryPad()]
                ->getTutorial()
                ->showTutorialPopup(false);
        // Temporarily make this scene fullscreen
        CXuiSceneBase::SetPlayerBaseScenePosition(
            ProfileManager.GetPrimaryPad(),
            CXuiSceneBase::e_BaseScene_Fullscreen
        );

        app.CloseXuiScenesAndNavigateToScene(
            ProfileManager.GetPrimaryPad(),
            eUIScene_EndPoem
        );
#else
        app.DebugPrintf(
            "handleGameEvent packet for WIN_GAME - %d\n",
            m_userIndex
        );
        // This just allows it to be shown
        if (minecraft->localgameModes[ProfileManager.GetPrimaryPad()] != NULL)
            minecraft->localgameModes[ProfileManager.GetPrimaryPad()]
                ->getTutorial()
                ->showTutorialPopup(false);
        ui.NavigateToScene(
            ProfileManager.GetPrimaryPad(),
            eUIScene_EndPoem,
            NULL,
            eUILayer_Scene,
            eUIGroup_Fullscreen
        );
#endif
    } else if (event == GameEventPacket::START_SAVING) {
        if (!g_NetworkManager.IsHost()) {
            // Move app started to here so that it happens immediately otherwise
            // back-to-back START/STOP packets leave the client stuck in the
            // loading screen
            app.SetGameStarted(false);
            app.SetAction(
                ProfileManager.GetPrimaryPad(),
                eAppAction_RemoteServerSave
            );
        }
    } else if (event == GameEventPacket::STOP_SAVING) {
        if (!g_NetworkManager.IsHost()) app.SetGameStarted(true);
    }
}

void ClientConnection::handleComplexItemData(
    std::shared_ptr<ComplexItemDataPacket> packet
) {
    if (packet->itemType == Item::map->id) {
        MapItem::getSavedData(packet->itemId, minecraft->level)
            ->handleComplexItemData(packet->data);
    } else {
        //		System.out.println("Unknown itemid: " + packet->itemId);	//
        // 4J removed
    }
}


void ClientConnection::handleLevelEvent(
    std::shared_ptr<LevelEventPacket> packet
) {
    switch (packet->type) {
    case LevelEvent::SOUND_DRAGON_DEATH: {
        Minecraft* pMinecraft = Minecraft::GetInstance();
        for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
            if (pMinecraft->localplayers[i] != NULL
                && pMinecraft->localplayers[i]->level != NULL
                && pMinecraft->localplayers[i]->level->dimension->id == 1) {
                pMinecraft->localplayers[i]->awardStat(
                    GenericStats::completeTheEnd(),
                    GenericStats::param_noArgs()
                );
            }
        }
    }
        minecraft->level->levelEvent(
            packet->type,
            packet->x,
            packet->y,
            packet->z,
            packet->data
        );

        break;
    default:
        minecraft->level->levelEvent(
            packet->type,
            packet->x,
            packet->y,
            packet->z,
            packet->data
        );
        break;
    }
}

void ClientConnection::handleAwardStat(
    std::shared_ptr<AwardStatPacket> packet
) {
    minecraft->localplayers[m_userIndex]->awardStatFromServer(
        GenericStats::stat(packet->statId),
        packet->getParamData()
    );
}

void ClientConnection::handleUpdateMobEffect(
    std::shared_ptr<UpdateMobEffectPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->entityId);
    if (e == NULL || std::dynamic_pointer_cast<Mob>(e) == NULL) return;

    (std::dynamic_pointer_cast<Mob>(e))
        ->addEffect(new MobEffectInstance(
            packet->effectId,
            packet->effectDurationTicks,
            packet->effectAmplifier
        ));
}

void ClientConnection::handleRemoveMobEffect(
    std::shared_ptr<RemoveMobEffectPacket> packet
) {
    std::shared_ptr<Entity> e = getEntity(packet->entityId);
    if (e == NULL || std::dynamic_pointer_cast<Mob>(e) == NULL) return;

    (std::dynamic_pointer_cast<Mob>(e))->removeEffectNoUpdate(packet->effectId);
}

bool ClientConnection::isServerPacketListener() { return false; }

void ClientConnection::handlePlayerInfo(
    std::shared_ptr<PlayerInfoPacket> packet
) {
    unsigned int startingPrivileges =
        app.GetPlayerPrivileges(packet->m_networkSmallId);

    INetworkPlayer* networkPlayer =
        g_NetworkManager.GetPlayerBySmallId(packet->m_networkSmallId);

    if (networkPlayer != NULL && networkPlayer->IsHost()) {
        // Some settings should always be considered on for the host player
        Player::enableAllPlayerPrivileges(startingPrivileges, true);
        Player::setPlayerGamePrivilege(
            startingPrivileges,
            Player::ePlayerGamePrivilege_HOST,
            1
        );
    }

    // 4J Stu - Repurposed this packet for player info that we want
    app.UpdatePlayerInfo(
        packet->m_networkSmallId,
        packet->m_playerColourIndex,
        packet->m_playerPrivileges
    );

    std::shared_ptr<Entity> entity = getEntity(packet->m_entityId);
    if (entity != NULL && entity->GetType() == eTYPE_PLAYER) {
        std::shared_ptr<Player> player =
            std::dynamic_pointer_cast<Player>(entity);
        player->setPlayerGamePrivilege(
            Player::ePlayerGamePrivilege_All,
            packet->m_playerPrivileges
        );
    }
    if (networkPlayer != NULL && networkPlayer->IsLocal()) {
        for (unsigned int i = 0; i < XUSER_MAX_COUNT; ++i) {
            std::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                minecraft->localplayers[i];
            if (localPlayer != NULL && localPlayer->connection != NULL
                && localPlayer->connection->getNetworkPlayer()
                       == networkPlayer) {
                localPlayer->setPlayerGamePrivilege(
                    Player::ePlayerGamePrivilege_All,
                    packet->m_playerPrivileges
                );
                displayPrivilegeChanges(localPlayer, startingPrivileges);
                break;
            }
        }
    }

    // 4J Stu - I don't think we care about this, so not converting it (came
    // from 1.8.2)
#if 0
	PlayerInfo pi = playerInfoMap.get(packet.name);
	if (pi == null && packet.add) {
		pi = new PlayerInfo(packet.name);
		playerInfoMap.put(packet.name, pi);
		playerInfos.add(pi);
	}
	if (pi != null && !packet.add) {
		playerInfoMap.remove(packet.name);
		playerInfos.remove(pi);
	}
	if (packet.add && pi != null) {
		pi.latency = packet.latency;
	}
#endif
}


void ClientConnection::displayPrivilegeChanges(
    std::shared_ptr<MultiplayerLocalPlayer> player,
    unsigned int                            oldPrivileges
) {
    int          userIndex               = player->GetXboxPad();
    unsigned int newPrivileges           = player->getAllPlayerGamePrivileges();
    Player::EPlayerGamePrivileges priv   = (Player::EPlayerGamePrivileges)0;
    bool                          privOn = false;
    for (unsigned int i = 0; i < Player::ePlayerGamePrivilege_MAX; ++i) {
        priv = (Player::EPlayerGamePrivileges)i;
        if (Player::getPlayerGamePrivilege(newPrivileges, priv)
            != Player::getPlayerGamePrivilege(oldPrivileges, priv)) {
            privOn = Player::getPlayerGamePrivilege(newPrivileges, priv);
            std::wstring message = L"";
            if (app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) {
                switch (priv) {
                case Player::ePlayerGamePrivilege_CannotMine:
                    if (privOn)
                        message = app.GetString(IDS_PRIV_MINE_TOGGLE_ON);
                    else message = app.GetString(IDS_PRIV_MINE_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CannotBuild:
                    if (privOn)
                        message = app.GetString(IDS_PRIV_BUILD_TOGGLE_ON);
                    else message = app.GetString(IDS_PRIV_BUILD_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches:
                    if (privOn)
                        message = app.GetString(IDS_PRIV_USE_DOORS_TOGGLE_ON);
                    else message = app.GetString(IDS_PRIV_USE_DOORS_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CanUseContainers:
                    if (privOn)
                        message =
                            app.GetString(IDS_PRIV_USE_CONTAINERS_TOGGLE_ON);
                    else
                        message =
                            app.GetString(IDS_PRIV_USE_CONTAINERS_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CannotAttackAnimals:
                    if (privOn)
                        message =
                            app.GetString(IDS_PRIV_ATTACK_ANIMAL_TOGGLE_ON);
                    else
                        message =
                            app.GetString(IDS_PRIV_ATTACK_ANIMAL_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CannotAttackMobs:
                    if (privOn)
                        message = app.GetString(IDS_PRIV_ATTACK_MOB_TOGGLE_ON);
                    else
                        message = app.GetString(IDS_PRIV_ATTACK_MOB_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CannotAttackPlayers:
                    if (privOn)
                        message =
                            app.GetString(IDS_PRIV_ATTACK_PLAYER_TOGGLE_ON);
                    else
                        message =
                            app.GetString(IDS_PRIV_ATTACK_PLAYER_TOGGLE_OFF);
                    break;
                };
            }
            switch (priv) {
            case Player::ePlayerGamePrivilege_Op:
                if (privOn)
                    message = app.GetString(IDS_PRIV_MODERATOR_TOGGLE_ON);
                else message = app.GetString(IDS_PRIV_MODERATOR_TOGGLE_OFF);
                break;
            };
            if (app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0) {
                switch (priv) {
                case Player::ePlayerGamePrivilege_CanFly:
                    if (privOn) message = app.GetString(IDS_PRIV_FLY_TOGGLE_ON);
                    else message = app.GetString(IDS_PRIV_FLY_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_ClassicHunger:
                    if (privOn)
                        message = app.GetString(IDS_PRIV_EXHAUSTION_TOGGLE_ON);
                    else
                        message = app.GetString(IDS_PRIV_EXHAUSTION_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_Invisible:
                    if (privOn)
                        message = app.GetString(IDS_PRIV_INVISIBLE_TOGGLE_ON);
                    else message = app.GetString(IDS_PRIV_INVISIBLE_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_Invulnerable:
                    if (privOn)
                        message =
                            app.GetString(IDS_PRIV_INVULNERABLE_TOGGLE_ON);
                    else
                        message =
                            app.GetString(IDS_PRIV_INVULNERABLE_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CanToggleInvisible:
                    if (privOn)
                        message =
                            app.GetString(IDS_PRIV_CAN_INVISIBLE_TOGGLE_ON);
                    else
                        message =
                            app.GetString(IDS_PRIV_CAN_INVISIBLE_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CanToggleFly:
                    if (privOn)
                        message = app.GetString(IDS_PRIV_CAN_FLY_TOGGLE_ON);
                    else message = app.GetString(IDS_PRIV_CAN_FLY_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CanToggleClassicHunger:
                    if (privOn)
                        message =
                            app.GetString(IDS_PRIV_CAN_EXHAUSTION_TOGGLE_ON);
                    else
                        message =
                            app.GetString(IDS_PRIV_CAN_EXHAUSTION_TOGGLE_OFF);
                    break;
                case Player::ePlayerGamePrivilege_CanTeleport:
                    if (privOn)
                        message =
                            app.GetString(IDS_PRIV_CAN_TELEPORT_TOGGLE_ON);
                    else
                        message =
                            app.GetString(IDS_PRIV_CAN_TELEPORT_TOGGLE_OFF);
                    break;
                };
            }
            if (!message.empty())
                minecraft->gui->addMessage(message, userIndex);
        }
    }
}

void ClientConnection::handleKeepAlive(
    std::shared_ptr<KeepAlivePacket> packet
) {
    send(std::shared_ptr<KeepAlivePacket>(new KeepAlivePacket(packet->id)));
}

void ClientConnection::handlePlayerAbilities(
    std::shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket
) {
    std::shared_ptr<Player> player = minecraft->localplayers[m_userIndex];
    player->abilities.flying       = playerAbilitiesPacket->isFlying();
    player->abilities.instabuild   = playerAbilitiesPacket->canInstabuild();
    player->abilities.invulnerable = playerAbilitiesPacket->isInvulnerable();
    player->abilities.mayfly       = playerAbilitiesPacket->canFly();
    player->abilities.setFlyingSpeed(playerAbilitiesPacket->getFlyingSpeed());
}

void ClientConnection::handleSoundEvent(
    std::shared_ptr<LevelSoundPacket> packet
) {
    minecraft->level->playLocalSound(
        packet->getX(),
        packet->getY(),
        packet->getZ(),
        packet->getSound(),
        packet->getVolume(),
        packet->getPitch()
    );
}

void ClientConnection::handleCustomPayload(
    std::shared_ptr<CustomPayloadPacket> customPayloadPacket
) {
    if (CustomPayloadPacket::TRADER_LIST_PACKET.compare(
            customPayloadPacket->identifier
        )
        == 0) {
        ByteArrayInputStream bais(customPayloadPacket->data);
        DataInputStream      input(&bais);
        int                  containerId = input.readInt();
        if (ui.IsSceneInStack(m_userIndex, eUIScene_TradingMenu)
            && containerId
                   == minecraft->localplayers[m_userIndex]
                          ->containerMenu->containerId) {
            std::shared_ptr<Merchant> trader = nullptr;

#ifdef _XBOX
            HXUIOBJ   scene     = app.GetCurrentScene(m_userIndex);
            HXUICLASS thisClass = XuiFindClass(L"CXuiSceneTrading");
            HXUICLASS objClass  = XuiGetObjectClass(scene);

            // Also returns TRUE if they are the same (which is what we want)
            if (XuiClassDerivesFrom(objClass, thisClass)) {
                CXuiSceneTrading* screen;
                HRESULT hr = XuiObjectFromHandle(scene, (void**)&screen);
                if (FAILED(hr)) return;
                trader = screen->getMerchant();
            }
#else
            UIScene* scene = ui.GetTopScene(m_userIndex, eUILayer_Scene);
            UIScene_TradingMenu* screen = (UIScene_TradingMenu*)scene;
            trader                      = screen->getMerchant();
#endif

            MerchantRecipeList* recipeList =
                MerchantRecipeList::createFromStream(&input);
            trader->overrideOffers(recipeList);
        }
    }
}

Connection* ClientConnection::getConnection() { return connection; }

// 4J Added
void ClientConnection::handleServerSettingsChanged(
    std::shared_ptr<ServerSettingsChangedPacket> packet
) {
    if (packet->action == ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS) {
        app.SetGameHostOption(eGameHostOption_All, packet->data);
    } else if (packet->action == ServerSettingsChangedPacket::HOST_DIFFICULTY) {
        for (unsigned int i = 0; i < minecraft->levels.length; ++i) {
            if (minecraft->levels[i] != NULL) {
                app.DebugPrintf(
                    "ClientConnection::handleServerSettingsChanged - "
                    "Difficulty = %d",
                    packet->data
                );
                minecraft->levels[i]->difficulty = packet->data;
            }
        }
    } else {
        // options
        // minecraft->options->SetGamertagSetting((packet->data==0)?false:true);
        app.SetGameHostOption(eGameHostOption_Gamertags, packet->data);
    }
}

void ClientConnection::handleXZ(std::shared_ptr<XZPacket> packet) {
    if (packet->action == XZPacket::STRONGHOLD) {
        minecraft->levels[0]->getLevelData()->setXStronghold(packet->x);
        minecraft->levels[0]->getLevelData()->setZStronghold(packet->z);
        minecraft->levels[0]->getLevelData()->setHasStronghold();
    }
}

void ClientConnection::handleUpdateProgress(
    std::shared_ptr<UpdateProgressPacket> packet
) {
    if (!g_NetworkManager.IsHost())
        Minecraft::GetInstance()->progressRenderer->progressStagePercentage(
            packet->m_percentage
        );
}

void ClientConnection::handleUpdateGameRuleProgressPacket(
    std::shared_ptr<UpdateGameRuleProgressPacket> packet
) {
    LPCWSTR string = app.GetGameRulesString(packet->m_messageId);
    if (string != NULL) {
        std::wstring message(string);
        message = GameRuleDefinition::generateDescriptionString(
            packet->m_definitionType,
            message,
            packet->m_data.data,
            packet->m_data.length
        );
        if (minecraft->localgameModes[m_userIndex] != NULL) {
            minecraft->localgameModes[m_userIndex]->getTutorial()->setMessage(
                message,
                packet->m_icon,
                packet->m_auxValue
            );
        }
    }
    // If this rule has a data tag associated with it, then we save that in user
    // profile data
    if (packet->m_dataTag > 0 && packet->m_dataTag <= 32) {
        app.DebugPrintf(
            "handleUpdateGameRuleProgressPacket: Data tag is in range, so "
            "updating profile data\n"
        );
        app.SetSpecialTutorialCompletionFlag(
            m_userIndex,
            packet->m_dataTag - 1
        );
    }
    delete[] packet->m_data.data;
}

// 4J Stu - TU-1 hotfix
// Fix for #13191 - The host of a game can get a message informing them that the
// connection to the server has been lost
int ClientConnection::HostDisconnectReturned(
    void*                      pParam,
    int                        iPad,
    C4JStorage::EMessageResult result
) {
    // 4J-PB - if they have a trial texture pack, they don't get to save the
    // world
    if (!Minecraft::GetInstance()->skins->isUsingDefaultSkin()) {
        TexturePack*    tPack = Minecraft::GetInstance()->skins->getSelected();
        DLCTexturePack* pDLCTexPack = (DLCTexturePack*)tPack;

        DLCPack* pDLCPack =
            pDLCTexPack->getDLCInfoParentPack(); // tPack->getDLCPack();
        if (!pDLCPack->hasPurchasedFile(DLCManager::e_DLCType_Texture, L"")) {
            // no upsell, we're about to quit
            MinecraftServer::getInstance()->setSaveOnExit(false);
            // flag a app action of exit game
            app.SetAction(iPad, eAppAction_ExitWorld);
        }
    }

#if defined(_XBOX_ONE) || defined(__ORBIS__)
    // Give the player the option to save their game
    // does the save exist?
    bool bSaveExists;
    StorageManager.DoesSaveExist(&bSaveExists);
    // 4J-PB - we check if the save exists inside the libs
    // we need to ask if they are sure they want to overwrite the existing game
    if (bSaveExists && StorageManager.GetSaveDisabled()) {
        UINT uiIDA[2];
        uiIDA[0] = IDS_CONFIRM_CANCEL;
        uiIDA[1] = IDS_CONFIRM_OK;
        ui.RequestMessageBox(
            IDS_TITLE_SAVE_GAME,
            IDS_CONFIRM_SAVE_GAME,
            uiIDA,
            2,
            ProfileManager.GetPrimaryPad(),
            &ClientConnection::ExitGameAndSaveReturned,
            NULL,
            app.GetStringTable()
        );
    } else
#else
    // Give the player the option to save their game
    // does the save exist?
    bool bSaveExists;
    StorageManager.DoesSaveExist(&bSaveExists);
    // 4J-PB - we check if the save exists inside the libs
    // we need to ask if they are sure they want to overwrite the existing game
    if (bSaveExists) {
        UINT uiIDA[2];
        uiIDA[0] = IDS_CONFIRM_CANCEL;
        uiIDA[1] = IDS_CONFIRM_OK;
        ui.RequestMessageBox(
            IDS_TITLE_SAVE_GAME,
            IDS_CONFIRM_SAVE_GAME,
            uiIDA,
            2,
            ProfileManager.GetPrimaryPad(),
            &ClientConnection::ExitGameAndSaveReturned,
            NULL,
            app.GetStringTable()
        );
    } else
#endif
    {
#if defined(_XBOX_ONE) || defined(__ORBIS__)
        StorageManager.SetSaveDisabled(false);
#endif
        MinecraftServer::getInstance()->setSaveOnExit(true);
        // flag a app action of exit game
        app.SetAction(iPad, eAppAction_ExitWorld);
    }

    return 0;
}

int ClientConnection::ExitGameAndSaveReturned(
    void*                      pParam,
    int                        iPad,
    C4JStorage::EMessageResult result
) {
    // results switched for this dialog
    if (result == C4JStorage::EMessage_ResultDecline) {
        // INT saveOrCheckpointId = 0;
        // bool validSave =
        // StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
        // SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(),
        // saveOrCheckpointId);
#if defined(_XBOX_ONE) || defined(__ORBIS__)
        StorageManager.SetSaveDisabled(false);
#endif
        MinecraftServer::getInstance()->setSaveOnExit(true);
    } else {
        MinecraftServer::getInstance()->setSaveOnExit(false);
    }
    // flag a app action of exit game
    app.SetAction(iPad, eAppAction_ExitWorld);
    return 0;
}

//
std::wstring ClientConnection::GetDisplayNameByGamertag(std::wstring gamertag) {
#ifdef _DURANGO
    std::wstring displayName =
        g_NetworkManager.GetDisplayNameByGamertag(gamertag);
    return displayName;
#else
    return gamertag;
#endif
}
