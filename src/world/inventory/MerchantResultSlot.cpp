#include "MerchantResultSlot.h"

#include <cstddef>

#include "world/entity/player/Player.h"
#include "world/item/ItemInstance.h"
#include "world/item/trading/Merchant.h"
#include "world/item/trading/MerchantRecipe.h"

#include "MerchantContainer.h"
#include "MerchantMenu.h"

MerchantResultSlot::MerchantResultSlot(
    Player*                            player,
    std::shared_ptr<Merchant>          merchant,
    std::shared_ptr<MerchantContainer> slots,
    int                                id,
    int                                x,
    int                                y
)
: Slot(slots, id, x, y) {
    this->player   = player;
    this->merchant = merchant;
    this->slots    = slots;
    removeCount    = 0;
}

bool MerchantResultSlot::mayPlace(std::shared_ptr<ItemInstance> item) {
    return false;
}

std::shared_ptr<ItemInstance> MerchantResultSlot::remove(int c) {
    if (hasItem()) {
        removeCount += std::min(c, getItem()->count);
    }
    return Slot::remove(c);
}

void MerchantResultSlot::onQuickCraft(
    std::shared_ptr<ItemInstance> picked,
    int                           count
) {
    removeCount += count;
    checkTakeAchievements(picked);
}

void MerchantResultSlot::checkTakeAchievements(
    std::shared_ptr<ItemInstance> carried
) {
    carried->onCraftedBy(
        player->level,
        std::dynamic_pointer_cast<Player>(player->shared_from_this()),
        removeCount
    );
    removeCount = 0;
}

void MerchantResultSlot::onTake(
    std::shared_ptr<Player>       player,
    std::shared_ptr<ItemInstance> carried
) {
    checkTakeAchievements(carried);

    MerchantRecipe* activeRecipe = slots->getActiveRecipe();
    if (activeRecipe != NULL) {
        std::shared_ptr<ItemInstance> item1 =
            slots->getItem(MerchantMenu::PAYMENT1_SLOT);
        std::shared_ptr<ItemInstance> item2 =
            slots->getItem(MerchantMenu::PAYMENT2_SLOT);

        // remove payment items, but remember slots may have switched
        if (removePaymentItemsIfMatching(activeRecipe, item1, item2)
            || removePaymentItemsIfMatching(activeRecipe, item2, item1)) {
            merchant->notifyTrade(activeRecipe);

            if (item1 && item1->count <= 0) {
                item1 = nullptr;
            }
            if (item2 && item2->count <= 0) {
                item2 = nullptr;
            }
            slots->setItem(MerchantMenu::PAYMENT1_SLOT, item1);
            slots->setItem(MerchantMenu::PAYMENT2_SLOT, item2);
        }
    }
}

bool MerchantResultSlot::mayCombine(std::shared_ptr<ItemInstance> second) {
    return false;
}

bool MerchantResultSlot::removePaymentItemsIfMatching(
    MerchantRecipe*               activeRecipe,
    std::shared_ptr<ItemInstance> a,
    std::shared_ptr<ItemInstance> b
) {
    std::shared_ptr<ItemInstance> buyA = activeRecipe->getBuyAItem();
    std::shared_ptr<ItemInstance> buyB = activeRecipe->getBuyBItem();

    if (a != NULL && a->id == buyA->id) {
        if (buyB != NULL && b != NULL && buyB->id == b->id) {
            a->count -= buyA->count;
            b->count -= buyB->count;
            return true;
        } else if (buyB == NULL && b == NULL) {
            a->count -= buyA->count;
            return true;
        }
    }
    return false;
}
