#include "AddEnchantmentRuleDefinition.h"

#include <cstddef>

#include "util/console/StringHelpers.h"
#include "util/java/InputOutputStream/DataOutputStream.h"
#include "win/Windows64_App.h"
#include "world/item/EnchantedBookItem.h"
#include "world/item/Item.h"
#include "world/item/ItemInstance.h"
#include "world/item/enchantment/Enchantment.h"
#include "world/item/enchantment/EnchantmentCategory.h"
#include "world/item/enchantment/EnchantmentInstance.h"

#include "GameRuleDefinition.h"

AddEnchantmentRuleDefinition::AddEnchantmentRuleDefinition() {
    m_enchantmentId = m_enchantmentLevel = 0;
}

void AddEnchantmentRuleDefinition::writeAttributes(
    DataOutputStream* dos,
    UINT              numAttributes
) {
    GameRuleDefinition::writeAttributes(dos, numAttributes + 2);

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_enchantmentId);
    dos->writeUTF(_toString(m_enchantmentId));

    ConsoleGameRules::write(
        dos,
        ConsoleGameRules::eGameRuleAttr_enchantmentLevel
    );
    dos->writeUTF(_toString(m_enchantmentLevel));
}

void AddEnchantmentRuleDefinition::addAttribute(
    const std::wstring& attributeName,
    const std::wstring& attributeValue
) {
    if (attributeName.compare(L"enchantmentId") == 0) {
        int value = _fromString<int>(attributeValue);
        if (value < 0) value = 0;
        if (value >= 256) value = 255;
        m_enchantmentId = value;
        app.DebugPrintf(
            "AddEnchantmentRuleDefinition: Adding parameter enchantmentId=%d\n",
            m_enchantmentId
        );
    } else if (attributeName.compare(L"enchantmentLevel") == 0) {
        int value = _fromString<int>(attributeValue);
        if (value < 0) value = 0;
        m_enchantmentLevel = value;
        app.DebugPrintf(
            "AddEnchantmentRuleDefinition: Adding parameter "
            "enchantmentLevel=%d\n",
            m_enchantmentLevel
        );
    } else {
        GameRuleDefinition::addAttribute(attributeName, attributeValue);
    }
}

bool AddEnchantmentRuleDefinition::enchantItem(
    std::shared_ptr<ItemInstance> item
) {
    bool enchanted = false;
    if (item != NULL) {
        // 4J-JEV: Ripped code from enchantmenthelpers
        // Maybe we want to add an addEnchantment method to EnchantmentHelpers
        if (item->id == Item::enchantedBook_Id) {
            Item::enchantedBook->addEnchantment(
                item,
                new EnchantmentInstance(m_enchantmentId, m_enchantmentLevel)
            );
        } else if (item->isEnchantable()) {
            Enchantment* e = Enchantment::enchantments[m_enchantmentId];

            if (e != NULL && e->category->canEnchant(item->getItem())) {
                int level = std::min(e->getMaxLevel(), m_enchantmentLevel);
                item->enchant(e, m_enchantmentLevel);
                enchanted = true;
            }
        }
    }
    return enchanted;
}
