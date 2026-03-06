#pragma once

#include <type_traits>
#include <utility>
#include <vector>

#include "client/GuiMessage.h"
#include "win/extraX64.h"

#include "GuiComponent.h"

class ItemRenderer;
class Minecraft;
class Random;

class Gui : public GuiComponent {
private:
    // 4J-PB - this doesn't account for the safe zone, and the indent applied to
    // messages
    // static const int MAX_MESSAGE_WIDTH = 320;
    static const int        m_iMaxMessageWidth = 280;
    static ItemRenderer*    itemRenderer;
    std::vector<GuiMessage> guiMessages[XUSER_MAX_COUNT];
    Random*                 random;

    Minecraft* minecraft;

public:
    std::wstring selectedName;

private:
    int          tickCount;
    std::wstring overlayMessageString;
    int          overlayMessageTime;
    bool         animateOverlayMessageColor;

    // 4J Added
    float lastTickA;
    float fAlphaIncrementPerCent;

public:
    static float currentGuiBlendFactor; // 4J added
    static float currentGuiScaleFactor; // 4J added

    float progress;

    //    private DecimalFormat df = new DecimalFormat("##.00");

public:
    Gui(Minecraft* minecraft);

    void  render(float a, bool mouseFree, int xMouse, int yMouse);
    float tbr;

private:
    // void renderBossHealth(void);
    void renderPumpkin(int w, int h);
    void renderVignette(float br, int w, int h);
    void renderTp(float br, int w, int h);
    void renderSlot(int slot, int x, int y, float a);

public:
    void tick();
    void clearMessages(int iPad = -1);
    void addMessage(
        const std::wstring& string,
        int                 iPad,
        bool                bIsDeathMessage = false
    );
    void setNowPlaying(const std::wstring& string);
    void displayClientMessage(int messageId, int iPad);

    // 4J Added
    DWORD getMessagesCount(int iPad) { return (int)guiMessages[iPad].size(); }
    std::wstring getMessage(int iPad, DWORD index) {
        return guiMessages[iPad].at(index).string;
    }
    float getOpacity(int iPad, DWORD index);

    std::wstring getJukeboxMessage(int iPad) { return overlayMessageString; }
    float        getJukeboxOpacity(int iPad);

    // 4J Added
    void renderGraph(
        int           dataLength,
        int           dataPos,
        std::int64_t* dataA,
        float         dataAScale,
        int           dataAWarning,
        std::int64_t* dataB,
        float         dataBScale,
        int           dataBWarning
    );
    void renderStackedGraph(
        int dataPos,
        int dataLength,
        int dataSources,
        std::int64_t (*func)(unsigned int dataPos, unsigned int dataSource)
    );
};
