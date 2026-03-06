#pragma once

#include <vector>

#include "platform/gamerules/ConsoleGameRulesConstants.h"

#include "CompoundGameRuleDefinition.h"

class AABB;
class NamedAreaRuleDefinition;
class StringTable;

class LevelRuleset : public CompoundGameRuleDefinition {
private:
    std::vector<NamedAreaRuleDefinition*> m_areas;
    StringTable*                          m_stringTable;

public:
    LevelRuleset();
    ~LevelRuleset();

    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition*
    addChild(ConsoleGameRules::EGameRuleType ruleType);

    virtual ConsoleGameRules::EGameRuleType getActionType() {
        return ConsoleGameRules::eGameRuleType_LevelRules;
    }

    void    loadStringTable(StringTable* table);
    LPCWSTR getString(const std::wstring& key);

    AABB* getNamedArea(const std::wstring& areaName);

    StringTable* getStringTable() { return m_stringTable; }
};
