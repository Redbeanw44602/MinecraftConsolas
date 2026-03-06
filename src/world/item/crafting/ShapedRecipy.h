#pragma once

#include <memory>

#include "Recipy.h"

class CraftingContainer;
class ItemInstance;

class ShapedRecipy : public Recipy {
private:
    int            width, height, group;
    ItemInstance** recipeItems;
    ItemInstance*  result;
    bool           _keepTag;

public:
    const int resultId;

public:
    ShapedRecipy(
        int            width,
        int            height,
        ItemInstance** recipeItems,
        ItemInstance*  result,
        int            iGroup = Recipy::eGroupType_Decoration
    );

    virtual const ItemInstance* getResultItem();
    virtual const int           getGroup();
    virtual bool
    matches(std::shared_ptr<CraftingContainer> craftSlots, Level* level);

private:
    bool matches(
        std::shared_ptr<CraftingContainer> craftSlots,
        int                                xOffs,
        int                                yOffs,
        bool                               xFlip
    );

public:
    virtual std::shared_ptr<ItemInstance>
                  assemble(std::shared_ptr<CraftingContainer> craftSlots);
    virtual int   size();
    ShapedRecipy* keepTag();

    // 4J-PB - to return the items required to make a recipe
    virtual bool
        needs(int iRecipe);
    virtual void
        needs(INGREDIENTS_REQUIRED* pIngReq);
};

