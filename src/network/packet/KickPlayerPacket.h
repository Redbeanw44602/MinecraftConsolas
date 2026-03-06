#pragma once

#include <memory>

#include "Packet.h"

class KickPlayerPacket : public Packet,
                         public std::enable_shared_from_this<KickPlayerPacket> {
public:
    BYTE m_networkSmallId;

    KickPlayerPacket();
    KickPlayerPacket(BYTE networkSmallId);

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual int  getEstimatedSize();

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new KickPlayerPacket());
    }
    virtual int getId() { return 159; }
};
