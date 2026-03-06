#pragma once

#include <memory>
#include <vector>

#include <mss.h>

#include "client/skins/SkinBox.h"

#include "Packet.h"

class DLCSkinFile;

class TextureAndGeometryPacket
: public Packet,
  public std::enable_shared_from_this<TextureAndGeometryPacket> {
public:
    std::wstring textureName;
    DWORD        dwSkinID;
    PBYTE        pbData;
    DWORD        dwTextureBytes;
    SKIN_BOX*    BoxDataA;
    DWORD        dwBoxC;
    unsigned int uiAnimOverrideBitmask;

    TextureAndGeometryPacket();
    ~TextureAndGeometryPacket();
    TextureAndGeometryPacket(
        const std::wstring& textureName,
        PBYTE               pbData,
        DWORD               dwBytes
    );
    TextureAndGeometryPacket(
        const std::wstring& textureName,
        PBYTE               pbData,
        DWORD               dwBytes,
        DLCSkinFile*        pDLCSkinFile
    );
    TextureAndGeometryPacket(
        const std::wstring&     textureName,
        PBYTE                   pbData,
        DWORD                   dwBytes,
        std::vector<SKIN_BOX*>* pvSkinBoxes,
        unsigned int            uiAnimOverrideBitmask
    );

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual int  getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new TextureAndGeometryPacket());
    }
    virtual int getId() { return 160; }
};
