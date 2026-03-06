#pragma once

#include <memory>

#include "Packet.h"

class MobEffectInstance;

class UpdateMobEffectPacket
: public Packet,
  public std::enable_shared_from_this<UpdateMobEffectPacket> {
public:
    int   entityId;
    BYTE  effectId;
    char  effectAmplifier;
    short effectDurationTicks;

    UpdateMobEffectPacket();
    UpdateMobEffectPacket(int entityId, MobEffectInstance* effect);

    virtual void read(DataInputStream* dis);
    virtual void write(DataOutputStream* dos);
    virtual void handle(PacketListener* listener);
    virtual int  getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(std::shared_ptr<Packet> packet);

public:
    static std::shared_ptr<Packet> create() {
        return std::shared_ptr<Packet>(new UpdateMobEffectPacket());
    }
    virtual int getId() { return 41; }
};
