#include "FurnaceResultSlot.h"

#include <cmath>

#include "stats/GenericStats.h"
#include "util/java/JavaMath.h"
#include "world/entity/ExperienceOrb.h"
#include "world/entity/player/Player.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/crafting/FurnaceRecipes.h"
#include "world/level/Level.h"

class Container;

FurnaceResultSlot::FurnaceResultSlot(
    std::shared_ptr<Player>    player,
    std::shared_ptr<Container> container,
    int                        slot,
    int                        x,
    int                        y
)
: Slot(container, slot, x, y) {
    this->player = player;
    removeCount  = 0;
}

bool FurnaceResultSlot::mayPlace(std::shared_ptr<ItemInstance> item) {
    return false;
}

std::shared_ptr<ItemInstance> FurnaceResultSlot::remove(int c) {
    if (hasItem()) {
        removeCount += min(c, getItem()->count);
    }
    return Slot::remove(c);
}

void FurnaceResultSlot::onTake(
    std::shared_ptr<Player>       player,
    std::shared_ptr<ItemInstance> carried
) {
    checkTakeAchievements(carried);
    Slot::onTake(player, carried);
}

void FurnaceResultSlot::onQuickCraft(
    std::shared_ptr<ItemInstance> picked,
    int                           count
) {
    removeCount += count;
    checkTakeAchievements(picked);
}

bool FurnaceResultSlot::mayCombine(std::shared_ptr<ItemInstance> second) {
    return false;
}

void FurnaceResultSlot::checkTakeAchievements(
    std::shared_ptr<ItemInstance> carried
) {
    carried->onCraftedBy(player->level, player, removeCount);
    // spawn xp right on top of the player
    if (!player->level->isClientSide) {
        int   amount = removeCount;
        float value =
            FurnaceRecipes::getInstance()->getRecipeValue(carried->id);

        if (value == 0) {
            amount = 0;
        } else if (value < 1) {
            int baseValue = floor((float)amount * value);
            if (baseValue < ceil((float)amount * value)
                && (float)Math::random()
                       < (((float)amount * value) - baseValue)) {
                baseValue++;
            }
            amount = baseValue;
        }

        while (amount > 0) {
            int newCount  = ExperienceOrb::getExperienceValue(amount);
            amount       -= newCount;
            player->level->addEntity(
                std::shared_ptr<ExperienceOrb>(new ExperienceOrb(
                    player->level,
                    player->x,
                    player->y + .5,
                    player->z + .5,
                    newCount
                ))
            );
        }
    }

#ifdef _DURANGO
    if (!player->level->isClientSide && removeCount > 0) {
        player->awardStat(
            GenericStats::itemsSmelted(carried->id),
            GenericStats::param_itemsSmelted(
                carried->id,
                carried->getAuxValue(),
                removeCount
            )
        );
    }
#else
    if (carried->id == Item::ironIngot_Id)
        player->awardStat(
            GenericStats::acquireIron(),
            GenericStats::param_acquireIron()
        );
    if (carried->id == Item::fish_cooked_Id)
        player->awardStat(
            GenericStats::cookFish(),
            GenericStats::param_cookFish()
        );
#endif

    removeCount = 0;
}
