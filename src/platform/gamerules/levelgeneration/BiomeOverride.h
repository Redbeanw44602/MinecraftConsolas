#pragma once


#include <mss.h>

#include "platform/gamerules/ConsoleGameRulesConstants.h"
#include "platform/gamerules/levelrules/ruledefinitions/GameRuleDefinition.h"

class BiomeOverride : public GameRuleDefinition {
private:
    BYTE m_topTile;
    BYTE m_tile;
    int  m_biomeId;

public:
    BiomeOverride();

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_BiomeOverride;
    }

    virtual void writeAttributes(DataOutputStream* dos, UINT numAttrs);
    virtual void addAttribute(
        const std::wstring& attributeName,
        const std::wstring& attributeValue
    );

    bool isBiome(int id);
    void getTileValues(BYTE& tile, BYTE& topTile);
};
