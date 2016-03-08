#include "actorcomponent.h"
#include "../actor.h"
#include "../level.h"
#include <cassert>

namespace killme
{
    ActorComponent::ActorComponent()
        : owner_(nullptr)
        , isActive_(false)
        , enableTicking_(false)
        , enableBeginFrame_(false)
        , processes_()
    {
    }

    void ActorComponent::setOwnerActor(Actor* owner)
    {
        assert((!owner || !owner_) && "This component is already attached to an other actor.");
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

    void ActorComponent::activate()
    {
        if (enableTicking_)
        {
            processes_.emplace_back(getOwnerLevel().registerTickingComponent(*this));
        }
        if (enableBeginFrame_)
        {
            processes_.emplace_back(getOwnerLevel().registerOnBeginFrameComponent(*this));
        }
        isActive_ = true;
        onActivate();
    }

    void ActorComponent::deactivate()
    {
        onDeactivate();
        processes_.clear();
        isActive_ = false;
    }

    bool ActorComponent::isActive() const
    {
        return isActive_;
    }


    void ActorComponent::enableTicking()
    {
        enableTicking_ = true;
    }

    void ActorComponent::enableBeginFrame()
    {
        enableBeginFrame_ = true;
    }
}