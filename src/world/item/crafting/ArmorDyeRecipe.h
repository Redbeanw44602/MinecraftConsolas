#pragma once

#include <memory>

#include "world/item/ItemInstance.h"

#include "Recipy.h"

class CraftingContainer;

class ArmorDyeRecipe : public Recipy {
public:
    bool matches(std::shared_ptr<CraftingContainer> craftSlots, Level* level);

    // 4J Stu - Made static as we use this in a different way from the Java (but
    // needs to be a different name otherwise Orbis compiler complains
    static std::shared_ptr<ItemInstance>
    assembleDyedArmor(std::shared_ptr<CraftingContainer> craftSlots);
    std::shared_ptr<ItemInstance>
    assemble(std::shared_ptr<CraftingContainer> craftSlots);

    int                 size();
    const ItemInstance* getResultItem();


    virtual const int getGroup();

    // 4J-PB
    virtual bool
        needs(int iRecipe);
    virtual void
        needs(INGREDIENTS_REQUIRED* pIngReq);
};
