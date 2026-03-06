#include "TheEndPortal.h"

#include <cstddef>
#include <type_traits>
#include <utility>

#include "platform/App_enums.h"
#include "util/java/Random.h"
#include "win/Windows64_App.h"
#include "world/IconRegister.h"
#include "world/ParticleTypes.h"
#include "world/entity/Entity.h"
#include "world/entity/player/Player.h"
#include "world/level/Level.h"
#include "world/level/dimension/Dimension.h"
#include "world/level/storage/LevelData.h"
#include "world/level/tile/entity/TheEndPortalTileEntity.h"

class Material;

DWORD TheEndPortal::tlsIdx = TlsAlloc();

// 4J - allowAnywhere is a static in java, implementing as TLS here to make
// thread safe
bool TheEndPortal::allowAnywhere() { return (TlsGetValue(tlsIdx) != NULL); }

void TheEndPortal::allowAnywhere(bool set) {
    TlsSetValue(tlsIdx, (LPVOID)(set ? 1 : 0));
}

TheEndPortal::TheEndPortal(int id, Material* material)
: EntityTile(id, material, isSolidRender()) {
    this->setLightEmission(1.0f);
}

std::shared_ptr<TileEntity> TheEndPortal::newTileEntity(Level* level) {
    return std::shared_ptr<TileEntity>(new TheEndPortalTileEntity());
}

void TheEndPortal::updateShape(
    LevelSource*                level,
    int                         x,
    int                         y,
    int                         z,
    int                         forceData,
    std::shared_ptr<TileEntity> forceEntity
) // 4J added forceData, forceEntity param
{
    float r = 1 / 16.0f;
    this->setShape(0, 0, 0, 1, r, 1);
}

bool TheEndPortal::shouldRenderFace(
    LevelSource* level,
    int          x,
    int          y,
    int          z,
    int          face
) {
    if (face != 0) return false;
    return EntityTile::shouldRenderFace(level, x, y, z, face);
}

void TheEndPortal::addAABBs(
    Level*                  level,
    int                     x,
    int                     y,
    int                     z,
    AABB*                   box,
    AABBList*               boxes,
    std::shared_ptr<Entity> source
) {}

bool TheEndPortal::isSolidRender(bool isServerLevel) { return false; }

bool TheEndPortal::isCubeShaped() { return false; }

int TheEndPortal::getResourceCount(Random* random) { return 0; }

void TheEndPortal::entityInside(
    Level*                  level,
    int                     x,
    int                     y,
    int                     z,
    std::shared_ptr<Entity> entity
) {
    if (entity->riding == NULL && entity->rider.lock() == NULL) {
        if (std::dynamic_pointer_cast<Player>(entity) != NULL) {
            if (!level->isClientSide) {
                // 4J Stu - Update the level data position so that the
                // stronghold portal can be shown on the maps
                int x, z;
                x = z = 0;
                if (level->dimension == 0
                    && !level->getLevelData()->getHasStrongholdEndPortal()
                    && app.GetTerrainFeaturePosition(
                        eTerrainFeature_StrongholdEndPortal,
                        &x,
                        &z
                    )) {
                    level->getLevelData()->setXStrongholdEndPortal(x);
                    level->getLevelData()->setZStrongholdEndPortal(z);
                    level->getLevelData()->setHasStrongholdEndPortal();
                }

                (std::dynamic_pointer_cast<Player>(entity))->changeDimension(1);
            }
        }
    }
}

void TheEndPortal::animateTick(
    Level*  level,
    int     xt,
    int     yt,
    int     zt,
    Random* random
) {
    double x  = xt + random->nextFloat();
    double y  = yt + 0.8f;
    double z  = zt + random->nextFloat();
    double xa = 0;
    double ya = 0;
    double za = 0;

    level->addParticle(eParticleType_endportal, x, y, z, xa, ya, za);
}

int TheEndPortal::getRenderShape() { return SHAPE_INVISIBLE; }

void TheEndPortal::onPlace(Level* level, int x, int y, int z) {
    if (allowAnywhere()) return;

    if (level->dimension->id != 0) {
        level->setTile(x, y, z, 0);
        return;
    }
}

int TheEndPortal::cloneTileId(Level* level, int x, int y, int z) { return 0; }

void TheEndPortal::registerIcons(IconRegister* iconRegister) {
    // don't register null, because of particles
    icon = iconRegister->registerIcon(L"portal");
}
