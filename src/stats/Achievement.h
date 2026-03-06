#pragma once

#include <memory>

#include "Stat.h"

class DescFormatter;
class Item;
class ItemInstance;
class Tile;

class Achievement : public Stat {
public:
    const int x, y;
    Achievement*
        needs;

private:
    const std::wstring desc;
    DescFormatter*     descFormatter;

public:
    const std::shared_ptr<ItemInstance> icon;

private:
    bool isGoldenVar;
    void _init();

public:
    Achievement(
        int                 id,
        const std::wstring& name,
        int                 x,
        int                 y,
        Item*               icon,
        Achievement* needs
    );
    Achievement(
        int                 id,
        const std::wstring& name,
        int                 x,
        int                 y,
        Tile*               icon,
        Achievement* needs
    );
    Achievement(
        int                           id,
        const std::wstring&           name,
        int                           x,
        int                           y,
        std::shared_ptr<ItemInstance> icon,
        Achievement* needs
    );

    Achievement* setAwardLocallyOnly();
    Achievement* setGolden();
    Achievement* postConstruct();
    bool         isAchievement();
    std::wstring getDescription();
    Achievement* setDescFormatter(DescFormatter* descFormatter);
    bool         isGolden();
    int          getAchievementID();
};
