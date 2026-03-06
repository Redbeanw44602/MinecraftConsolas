#include "SignRenderer.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <mss.h>

#include "client/Minecraft.h"
#include "client/gui/Font.h"
#include "client/model/SignModel.h"
#include "client/model/geom/ModelPart.h"
#include "client/renderer/Stubs.h"
#include "client/renderer/Textures.h"
#include "platform/App_enums.h"
#include "platform/colours/ColourTable.h"
#include "win/Windows64_App.h"
#include "win/extraX64.h"
#include "win/strings.h"
#include "world/level/tile/Tile.h"
#include "world/level/tile/entity/SignTileEntity.h"
#include "world/level/tile/entity/TileEntity.h"

SignRenderer::SignRenderer() { signModel = new SignModel(); }

void SignRenderer::render(
    std::shared_ptr<TileEntity> _sign,
    double                      x,
    double                      y,
    double                      z,
    float                       a,
    bool                        setColor,
    float                       alpha,
    bool                        useCompiled
) {
    // 4J - dynamic cast required because we aren't using templates/generics in
    // our version
    std::shared_ptr<SignTileEntity> sign =
        std::dynamic_pointer_cast<SignTileEntity>(_sign);

    Tile* tile = sign->getTile();

    glPushMatrix();
    float size = 16 / 24.0f;
    if (tile == Tile::sign) {
        glTranslatef((float)x + 0.5f, (float)y + 0.75f * size, (float)z + 0.5f);
        float rot = sign->getData() * 360 / 16.0f;
        glRotatef(-rot, 0, 1, 0);
        signModel->cube2->visible = true;
    } else {
        int   face = sign->getData();
        float rot  = 0;

        if (face == 2) rot = 180;
        if (face == 4) rot = 90;
        if (face == 5) rot = -90;

        glTranslatef((float)x + 0.5f, (float)y + 0.75f * size, (float)z + 0.5f);
        glRotatef(-rot, 0, 1, 0);
        glTranslatef(0, -5 / 16.0f, -7 / 16.0f);

        signModel->cube2->visible = false;
    }

    bindTexture(TN_ITEM_SIGN); // 4J was L"/item/sign.png"

    glPushMatrix();
    glScalef(size, -size, -size);
    signModel->render(true);
    glPopMatrix();
    Font* font = getFont();

    float s = 1 / 60.0f * size;
    glTranslatef(0, 0.5f * size, 0.07f * size);
    glScalef(s, -s, s);
    glNormal3f(0, 0, -1 * s);
    glDepthMask(false);

    int col = Minecraft::GetInstance()->getColourTable()->getColor(
        eMinecraftColour_Sign_Text
    );
    std::wstring msg;
    // need to send the new data
    // Get the current language setting from the console
    DWORD dwLanguage = XGetLanguage();

    for (int i = 0; i < MAX_SIGN_LINES; i++) // 4J - was sign.messages.length
    {
        if (sign->IsVerified()) {
            if (sign->IsCensored()) {
                switch (dwLanguage) {
                case XC_LANGUAGE_KOREAN:
                case XC_LANGUAGE_JAPANESE:
                case XC_LANGUAGE_TCHINESE:
                    msg = L"Censored"; // In-game font, so English only
                    break;
                default:
                    msg = app.GetString(IDS_STRINGVERIFY_CENSORED);
                    break;
                }
            } else {
                msg = sign->GetMessage(i);
            }
        } else {
            switch (dwLanguage) {
            case XC_LANGUAGE_KOREAN:
            case XC_LANGUAGE_JAPANESE:
            case XC_LANGUAGE_TCHINESE:
                msg = L"Awaiting Approval"; // In-game font, so English only
                break;
            default:
                msg = app.GetString(IDS_STRINGVERIFY_AWAITING_APPROVAL);
                break;
            }
        }

        if (i == sign->GetSelectedLine()) {
            msg = L"> " + msg + L" <";
            font->draw(
                msg,
                -font->width(msg) / 2,
                i * 10 - (MAX_SIGN_LINES) * 5,
                col
            ); // 4J - (MAX_SIGN_LINES) was sign.messages.length
        } else {
            font->draw(
                msg,
                -font->width(msg) / 2,
                i * 10 - (MAX_SIGN_LINES) * 5,
                col
            ); // 4J - (MAX_SIGN_LINES) was sign.messages.length
        }
    }
    glDepthMask(true);
    glColor4f(1, 1, 1, 1);
    glPopMatrix();
}
