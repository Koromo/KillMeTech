#include "actorcomponent.h"
#include "../actor.h"
#include "../level.h"
#include <cassert>

namespace killme
{
    void ActorComponent::setOwner(Actor* owner)
    {
        assert(!owner_ && "This component is already attached to an other actor.");
        owner_ = owner;
    }

    Actor& ActorComponent::getOwnerActor()
    {
        assert(owner_ && "No owner actor.");
        return *owner_;
    }

    Level& ActorComponent::getOwnerLevel()
    {
        return getOwnerActor().getOwnerLevel();
    }

    bool ActorComponent::hasOwner() const
    {
        return !!owner_;
    }

    void ActorComponent::activate()
    {
        KILLME_CONNECT_EVENT_HOOKS();
        if (tickable_)
        {
            tickingProcess_ = getOwnerLevel().registerTicking(*this);
        }
        onActivate();
        isActive_ = true;
    }

    void ActorComponent::deactivate()
    {
        onDeactivate();
        setTickable(false);
        KILLME_DISCONNECT_EVENT_HOOKS();
        isActive_ = false;
    }

    bool ActorComponent::isActive() const
    {
        return isActive_;
    }

    void ActorComponent::tick(float dt_s)
    {
        onTick(dt_s);
    }

    void ActorComponent::setTickable(bool enable)
    {
        if (tickable_ == enable)
        {
            return;
        }
        if (isActive_)
        {
            if (enable)
            {
                tickingProcess_ = getOwnerLevel().registerTicking(*this);
            }
            else
            {
                tickingProcess_.kill();
            }
        }

        tickable_ = enable;
    }

    ActorComponent::ActorComponent()
        : owner_(nullptr)
        , isActive_(false)
        , tickable_(false)
        , tickingProcess_()
    {
    }
}