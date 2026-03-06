#pragma once

#include <unordered_map>
#include <vector>

#include <mss.h>

#include "platform/gamerules/levelgeneration/LevelGenerators.h"
#include "platform/gamerules/levelrules/LevelRules.h"
#include "win/extraX64.h"

#include "ConsoleGameRulesConstants.h"

class DLCGameRulesHeader;
class DLCPack;
class DataInputStream;
class DataOutputStream;
class File;
class GameRuleDefinition;
class LevelChunk;
class LevelGenerationOptions;
class LevelRuleset;
class StringTable;

#define GAME_RULE_SAVENAME L"requiredGameRules.grf"

// 4J-JEV:
#define LEVEL_GEN_ID      int
#define LEVEL_GEN_ID_NULL 0

class GameRuleManager {
public:
    static WCHAR* wchTagNameA[ConsoleGameRules::eGameRuleType_Count];
    static WCHAR* wchAttrNameA[ConsoleGameRules::eGameRuleAttr_Count];

    static const short version_number = 2;

private:
    LevelGenerationOptions* m_currentLevelGenerationOptions;
    LevelRuleset*           m_currentGameRuleDefinitions;
    LevelGenerators         m_levelGenerators;
    LevelRules              m_levelRules;

public:
    GameRuleManager();

    void loadGameRules(DLCPack*);

    LevelGenerationOptions* loadGameRules(byte* dIn, UINT dSize);
    void loadGameRules(LevelGenerationOptions* lgo, byte* dIn, UINT dSize);

    void saveGameRules(byte** dOut, UINT* dSize);

private:
    LevelGenerationOptions* readHeader(DLCGameRulesHeader* grh);

    void writeRuleFile(DataOutputStream* dos);

public:
    bool readRuleFile(
        LevelGenerationOptions* lgo,
        byte*                   dIn,
        UINT                    dSize,
        StringTable*            strings
    ); //(DLCGameRulesFile *dlcFile, StringTable *strings);

private:
    void readAttributes(
        DataInputStream*           dis,
        std::vector<std::wstring>* tagsAndAtts,
        GameRuleDefinition*        rule
    );
    void readChildren(
        DataInputStream*                                          dis,
        std::vector<std::wstring>*                                tagsAndAtts,
        std::unordered_map<int, ConsoleGameRules::EGameRuleType>* tagIdMap,
        GameRuleDefinition*                                       rule
    );

public:
    void processSchematics(LevelChunk* levelChunk);
    void processSchematicsLighting(LevelChunk* levelChunk);
    void loadDefaultGameRules();

private:
    bool loadGameRulesPack(File* path);

    LEVEL_GEN_ID addLevelGenerationOptions(LevelGenerationOptions*);

public:
    std::vector<LevelGenerationOptions*>* getLevelGenerators() {
        return m_levelGenerators.getLevelGenerators();
    }
    void          setLevelGenerationOptions(LevelGenerationOptions* levelGen);
    LevelRuleset* getGameRuleDefinitions() {
        return m_currentGameRuleDefinitions;
    }
    LevelGenerationOptions* getLevelGenerationOptions() {
        return m_currentLevelGenerationOptions;
    }
    LPCWSTR GetGameRulesString(const std::wstring& key);

    // 4J-JEV:
    // Properly cleans-up and unloads the current set of gameRules.
    void unloadCurrentGameRules();
};
