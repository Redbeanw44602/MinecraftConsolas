#pragma once

#include "commands/CommandsEnum.h"
#include "world/entity/player/Player.h"

class Input;
class Level;

class RemotePlayer : public Player {
private:
    bool hasStartedUsingItem;

public:
    Input* input;
    RemotePlayer(Level* level, const std::wstring& name);

protected:
    virtual void setDefaultHeadHeight();

public:
    virtual bool hurt(DamageSource* source, int dmg);

private:
    int    lSteps;
    double lx, ly, lz, lyr, lxr;

public:
    virtual void
    lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
    float fallTime;

    virtual void  tick();
    virtual float getShadowHeightOffs();
    virtual void  aiStep();
    virtual void  setEquippedSlot(
         int                           slot,
         std::shared_ptr<ItemInstance> item
     ); // 4J Stu - Brought forward change from 1.3 to fix #64688 - Customer
       // Encountered: TU7: Content: Art: Aura of enchanted item is not
       // displayed for other players in online game
    virtual void  animateRespawn();
    virtual float getHeadHeight();
    bool          hasPermission(EGameCommand command) { return false; }
};
