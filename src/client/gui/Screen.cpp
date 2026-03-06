#include "Screen.h"

#include <cstddef>
#include <type_traits>

#include "client/Minecraft.h"
#include "client/gui/particle/GuiParticles.h"
#include "client/renderer/Stubs.h"
#include "platform/audio/SoundEngine.h"
#include "world/SoundTypes.h"

#include "Button.h"

Screen::Screen() // 4J added
{
    minecraft     = NULL;
    width         = 0;
    height        = 0;
    passEvents    = false;
    font          = NULL;
    particles     = NULL;
    clickedButton = NULL;
}

void Screen::render(int xm, int ym, float a) {
    auto itEnd = buttons.end();
    for (auto it = buttons.begin(); it != itEnd; it++) {
        Button* button = *it; // buttons[i];
        button->render(minecraft, xm, ym);
    }
}

void Screen::keyPressed(wchar_t eventCharacter, int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(NULL);
        //    minecraft->grabMouse();	// 4J - removed
    }
}

void Screen::setClipboard(const std::wstring& str) {
    // 4J - removed
}

void Screen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum == 0) {
        auto itEnd = buttons.end();
        for (auto it = buttons.begin(); it != itEnd; it++) {
            Button* button = *it; // buttons[i];
            if (button->clicked(minecraft, x, y)) {
                clickedButton = button;
                minecraft->soundEngine->playUI(eSoundType_RANDOM_CLICK, 1, 1);
                buttonClicked(button);
            }
        }
    }
}

void Screen::mouseReleased(int x, int y, int buttonNum) {
    if (clickedButton != NULL && buttonNum == 0) {
        clickedButton->released(x, y);
        clickedButton = NULL;
    }
}

void Screen::buttonClicked(Button* button) {}

void Screen::init(Minecraft* minecraft, int width, int height) {
    particles       = new GuiParticles(minecraft);
    this->minecraft = minecraft;
    this->font      = minecraft->font;
    this->width     = width;
    this->height    = height;
    buttons.clear();
    init();
}

void Screen::setSize(int width, int height) {
    this->width  = width;
    this->height = height;
}

void Screen::init() {}

void Screen::updateEvents() {
    /* 4J - TODO
    while (Mouse.next()) {
        mouseEvent();
    }

    while (Keyboard.next()) {
        keyboardEvent();
    }
    */
}

void Screen::mouseEvent() {
    /* 4J - TODO
    if (Mouse.getEventButtonState()) {
        int xm = Mouse.getEventX() * width / minecraft.width;
        int ym = height - Mouse.getEventY() * height / minecraft.height - 1;
        mouseClicked(xm, ym, Mouse.getEventButton());
    } else {
        int xm = Mouse.getEventX() * width / minecraft.width;
        int ym = height - Mouse.getEventY() * height / minecraft.height - 1;
        mouseReleased(xm, ym, Mouse.getEventButton());
    }
    */
}

void Screen::keyboardEvent() {
    /* 4J - TODO
    if (Keyboard.getEventKeyState()) {
        if (Keyboard.getEventKey() == Keyboard.KEY_F11) {
            minecraft.toggleFullScreen();
            return;
        }
        keyPressed(Keyboard.getEventCharacter(), Keyboard.getEventKey());
    }
    */
}

void Screen::tick() {}

void Screen::removed() {}

void Screen::renderBackground() { renderBackground(0); }

void Screen::renderBackground(int vo) {
    if (minecraft->level != NULL) {
        fillGradient(0, 0, width, height, 0xc0101010, 0xd0101010);
    } else {
        renderDirtBackground(vo);
    }
}

void Screen::renderDirtBackground(int vo) {
    // 4J Unused
#if 0
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    Tesselator *t = Tesselator::getInstance();
    glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadTexture(L"/gui/background.png"));
    glColor4f(1, 1, 1, 1);
    float s = 32;
    t->begin();
    t->color(0x404040);
    t->vertexUV((float)(0), (float)( height), (float)( 0), (float)( 0), (float)( height / s + vo));
    t->vertexUV((float)(width), (float)( height), (float)( 0), (float)( width / s), (float)( height / s + vo));
    t->vertexUV((float)(width), (float)( 0), (float)( 0), (float)( width / s), (float)( 0 + vo));
    t->vertexUV((float)(0), (float)( 0), (float)( 0), (float)( 0), (float)( 0 + vo));
    t->end();
#endif
}

bool Screen::isPauseScreen() { return true; }

void Screen::confirmResult(bool result, int id) {}

void Screen::tabPressed() {}
