#include "EatTileGoal.h"

#include <memory>

#include "util/Mth.h"
#include "util/java/Random.h"
#include "world/entity/EntityEvent.h"
#include "world/entity/Mob.h"
#include "world/entity/ai/control/Control.h"
#include "world/entity/ai/navigation/PathNavigation.h"
#include "world/level/Level.h"
#include "world/level/tile/LevelEvent.h"
#include "world/level/tile/TallGrass.h"
#include "world/level/tile/Tile.h"

EatTileGoal::EatTileGoal(Mob* mob) {
    eatAnimationTick = 0;

    this->mob   = mob;
    this->level = mob->level;
    setRequiredControlFlags(
        Control::MoveControlFlag | Control::LookControlFlag
        | Control::JumpControlFlag
    );
}

bool EatTileGoal::canUse() {
    if (mob->getRandom()->nextInt(mob->isBaby() ? 50 : 1000) != 0) return false;

    int xx = Mth::floor(mob->x);
    int yy = Mth::floor(mob->y);
    int zz = Mth::floor(mob->z);
    if (level->getTile(xx, yy, zz) == Tile::tallgrass_Id
        && level->getData(xx, yy, zz) == TallGrass::TALL_GRASS)
        return true;
    if (level->getTile(xx, yy - 1, zz) == Tile::grass_Id) return true;
    return false;
}

void EatTileGoal::start() {
    eatAnimationTick = EAT_ANIMATION_TICKS;
    level->broadcastEntityEvent(
        mob->shared_from_this(),
        EntityEvent::EAT_GRASS
    );
    mob->getNavigation()->stop();
}

void EatTileGoal::stop() { eatAnimationTick = 0; }

bool EatTileGoal::canContinueToUse() { return eatAnimationTick > 0; }

int EatTileGoal::getEatAnimationTick() { return eatAnimationTick; }

void EatTileGoal::tick() {
    eatAnimationTick = max(0, eatAnimationTick - 1);
    if (eatAnimationTick != 4) return;

    int xx = Mth::floor(mob->x);
    int yy = Mth::floor(mob->y);
    int zz = Mth::floor(mob->z);

    if (level->getTile(xx, yy, zz) == Tile::tallgrass_Id) {
        level->levelEvent(
            LevelEvent::PARTICLES_DESTROY_BLOCK,
            xx,
            yy,
            zz,
            Tile::tallgrass_Id + (TallGrass::TALL_GRASS << Tile::TILE_NUM_SHIFT)
        );
        level->setTile(xx, yy, zz, 0);
        mob->ate();
    } else if (level->getTile(xx, yy - 1, zz) == Tile::grass_Id) {
        level->levelEvent(
            LevelEvent::PARTICLES_DESTROY_BLOCK,
            xx,
            yy - 1,
            zz,
            Tile::grass_Id
        );
        level->setTile(xx, yy - 1, zz, Tile::dirt_Id);
        mob->ate();
    }
}
