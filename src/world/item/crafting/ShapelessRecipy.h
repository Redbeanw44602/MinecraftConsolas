#pragma once

#include <vector>

#include "Recipy.h"

class ItemInstance;

class ShapelessRecipy : public Recipy {
private:
    _eGroupType                 group;
    const ItemInstance*         result;
    std::vector<ItemInstance*>* ingredients;

public:
    ShapelessRecipy(
        ItemInstance*               result,
        std::vector<ItemInstance*>* ingredients,
        _eGroupType                 egroup = Recipy::eGroupType_Decoration
    );

    virtual const ItemInstance* getResultItem();
    virtual const int           getGroup();
    virtual bool
    matches(std::shared_ptr<CraftingContainer> craftSlots, Level* level);
    virtual std::shared_ptr<ItemInstance>
                assemble(std::shared_ptr<CraftingContainer> craftSlots);
    virtual int size();

    // 4J-PB - to return the items required to make a recipe
    virtual bool
        needs(int iRecipe);
    virtual void
        needs(INGREDIENTS_REQUIRED* pIngReq);
};
