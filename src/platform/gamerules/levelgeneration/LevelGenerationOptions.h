#pragma once

#pragma message("LevelGenerationOptions.h ")

#include <unordered_map>
#include <vector>

#include <mss.h>

#include "platform/gamerules/ConsoleGameRulesConstants.h"
#include "platform/gamerules/levelrules/ruledefinitions/GameRuleDefinition.h"
#include "world/level/levelgen/structure/StructureFeature.h"

class ApplySchematicRuleDefinition;
class BiomeOverride;
class ConsoleGenerateStructure;
class ConsoleSchematicFile;
class LevelChunk;
class LevelRuleset;
class Pos;
class StartFeature;
class StringTable;

class GrSource {
public:
    // 4J-JEV:
    // Moved all this here; I didn't like that all this header information
    // was being mixed in with all the game information as they have
    // completely different lifespans.

    virtual bool         requiresTexturePack()      = 0;
    virtual UINT         getRequiredTexturePackId() = 0;
    virtual std::wstring getDefaultSaveName()       = 0;
    virtual LPCWSTR      getWorldName()             = 0;
    virtual LPCWSTR      getDisplayName()           = 0;
    virtual std::wstring getGrfPath()               = 0;
    virtual bool         requiresBaseSave()         = 0;
    virtual std::wstring getBaseSavePath()          = 0;

    virtual void setRequiresTexturePack(bool)            = 0;
    virtual void setRequiredTexturePackId(UINT)          = 0;
    virtual void setDefaultSaveName(const std::wstring&) = 0;
    virtual void setWorldName(const std::wstring&)       = 0;
    virtual void setDisplayName(const std::wstring&)     = 0;
    virtual void setGrfPath(const std::wstring&)         = 0;
    virtual void setBaseSavePath(const std::wstring&)    = 0;

    virtual bool ready() = 0;

    // virtual void getGrfData(PBYTE &pData, DWORD &pSize)=0;
};

class JustGrSource : public GrSource {
protected:
    std::wstring m_worldName;
    std::wstring m_displayName;
    std::wstring m_defaultSaveName;
    bool         m_bRequiresTexturePack;
    int          m_requiredTexturePackId;
    std::wstring m_grfPath;
    std::wstring m_baseSavePath;
    bool         m_bRequiresBaseSave;

public:
    virtual bool         requiresTexturePack();
    virtual UINT         getRequiredTexturePackId();
    virtual std::wstring getDefaultSaveName();
    virtual LPCWSTR      getWorldName();
    virtual LPCWSTR      getDisplayName();
    virtual std::wstring getGrfPath();
    virtual bool         requiresBaseSave();
    virtual std::wstring getBaseSavePath();

    virtual void setRequiresTexturePack(bool x);
    virtual void setRequiredTexturePackId(UINT x);
    virtual void setDefaultSaveName(const std::wstring& x);
    virtual void setWorldName(const std::wstring& x);
    virtual void setDisplayName(const std::wstring& x);
    virtual void setGrfPath(const std::wstring& x);
    virtual void setBaseSavePath(const std::wstring& x);

    virtual bool ready();

    JustGrSource();
};

class LevelGenerationOptions : public GameRuleDefinition {
public:
    enum eSrc {
        eSrc_none,

        eSrc_fromSave, // Neither content or header is persistent.

        eSrc_fromDLC, // Header is persistent, content should be deleted to
                      // conserve space.

        eSrc_tutorial, // Both header and content is persistent, content cannot
                       // be reloaded.

        eSrc_MAX
    };

private:
    eSrc m_src;

    GrSource* m_pSrc;
    GrSource* info();

    bool m_hasLoadedData;

    PBYTE m_pbBaseSaveData;
    DWORD m_dwBaseSaveSize;

public:
    void setSrc(eSrc src);
    eSrc getSrc();

    bool isTutorial();
    bool isFromSave();
    bool isFromDLC();

    bool         requiresTexturePack();
    UINT         getRequiredTexturePackId();
    std::wstring getDefaultSaveName();
    LPCWSTR      getWorldName();
    LPCWSTR      getDisplayName();
    std::wstring getGrfPath();
    bool         requiresBaseSave();
    std::wstring getBaseSavePath();

    void setGrSource(GrSource* grs);

    void setRequiresTexturePack(bool x);
    void setRequiredTexturePackId(UINT x);
    void setDefaultSaveName(const std::wstring& x);
    void setWorldName(const std::wstring& x);
    void setDisplayName(const std::wstring& x);
    void setGrfPath(const std::wstring& x);
    void setBaseSavePath(const std::wstring& x);

    bool ready();

    void  setBaseSaveData(PBYTE pbData, DWORD dwSize);
    PBYTE getBaseSaveData(DWORD& size);
    bool  hasBaseSaveData();
    void  deleteBaseSaveData();

    bool hasLoadedData();
    void setLoadedData();

private:
    // This should match the "MapOptionsRule" definition in the XML schema
    std::int64_t                                            m_seed;
    bool                                                    m_useFlatWorld;
    Pos*                                                    m_spawnPos;
    std::vector<ApplySchematicRuleDefinition*>              m_schematicRules;
    std::vector<ConsoleGenerateStructure*>                  m_structureRules;
    bool                                                    m_bHaveMinY;
    int                                                     m_minY;
    std::unordered_map<std::wstring, ConsoleSchematicFile*> m_schematics;
    std::vector<BiomeOverride*>                             m_biomeOverrides;
    std::vector<StartFeature*>                              m_features;

    bool          m_bRequiresGameRules;
    LevelRuleset* m_requiredGameRules;

    StringTable* m_stringTable;

public:
    LevelGenerationOptions();
    ~LevelGenerationOptions();

    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual void writeAttributes(DataOutputStream* dos, UINT numAttributes);
    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual GameRuleDefinition*
                 addChild(ConsoleGameRules::EGameRuleType ruleType);
    virtual void addAttribute(
        const std::wstring& attributeName,
        const std::wstring& attributeValue
    );

    std::int64_t getLevelSeed();
    Pos*         getSpawnPos();
    bool         getuseFlatWorld();

    void processSchematics(LevelChunk* chunk);
    void processSchematicsLighting(LevelChunk* chunk);

    bool checkIntersects(int x0, int y0, int z0, int x1, int y1, int z1);

private:
    void clearSchematics();

public:
    ConsoleSchematicFile*
    loadSchematicFile(const std::wstring& filename, PBYTE pbData, DWORD dwLen);

public:
    ConsoleSchematicFile* getSchematicFile(const std::wstring& filename);
    void                  releaseSchematicFile(const std::wstring& filename);

    bool          requiresGameRules();
    void          setRequiredGameRules(LevelRuleset* rules);
    LevelRuleset* getRequiredGameRules();

    void getBiomeOverride(int biomeId, BYTE& tile, BYTE& topTile);
    bool isFeatureChunk(
        int                             chunkX,
        int                             chunkZ,
        StructureFeature::EFeatureTypes feature
    );

    void    loadStringTable(StringTable* table);
    LPCWSTR getString(const std::wstring& key);

    std::unordered_map<std::wstring, ConsoleSchematicFile*>*
    getUnfinishedSchematicFiles();

    // 4J-JEV:
    // ApplySchematicRules contain limited state
    // which needs to be reset BEFORE a new game starts.
    void reset_start();

    // 4J-JEV:
    // This file contains state that needs to be deleted
    // or reset once a game has finished.
    void reset_finish();
};
