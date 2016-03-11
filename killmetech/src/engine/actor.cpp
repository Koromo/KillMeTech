#include "actor.h"
#include "components/actorcomponent.h"
#include "components/transformcomponent.h"

namespace killme
{
    void Actor::spawned(Level* inLevel, const std::string& name)
    {
        inLevel_ = inLevel;
        name_ = name;
        onSpawn();
    }

    void Actor::despawned()
    {
        onDespawn();
        inLevel_ = nullptr;
        name_.clear();
    }

    Level& Actor::getOwnerLevel()
    {
        assert(inLevel_ && "No owner level.");
        return *inLevel_;
    }

    std::string Actor::getName() const
    {
        assert(inLevel_ && "No actor name.");
        return name_;
    }

    void Actor::activate()
    {
        if (isActive_ || !inLevel_)
        {
            return;
        }

        KILLME_CONNECT_EVENT_HOOKS();
        if (tickable_)
        {
            tickingProcess_ = getOwnerLevel().registerTicking(*this);
        }
        onActivate();
        isActive_ = true;

        for (const auto& c : conceptComponents_)
        {
            c->activate();
        }
        if (rootTransform_)
        {
            depthTraverse(*rootTransform_, [](TransformComponent& n)
            {
                n.activate();
                return true;
            });
        }
    }

    void Actor::deactivate()
    {
        if (!isActive_ || !inLevel_)
        {
            return;
        }

        onDeactivate();
        setTickable(false);
        KILLME_DISCONNECT_EVENT_HOOKS();
        isActive_ = false;

        for (const auto& c : conceptComponents_)
        {
            c->deactivate();
        }
        if (rootTransform_)
        {
            depthTraverse(*rootTransform_, [](TransformComponent& n)
            {
                n.deactivate();
                return true;
            });
        }
    }

    void Actor::attachConceptComponentImpl(const std::shared_ptr<ActorComponent>& component)
    {
        component->setOwner(this);
        componentMap_.emplace(component->getComponentType(), component);
    }

    void Actor::attachTransformedComponentImpl(const std::shared_ptr<TransformComponent>& component)
    {
        depthTraverse(component, [&](const std::shared_ptr<TransformComponent>& n)
        {
            n->setOwner(this);
            componentMap_.emplace(n->getComponentType(), n);
            return true;
        });
    }

    std::shared_ptr<TransformComponent> Actor::getRootTransform()
    {
        return rootTransform_;
    }

    void Actor::tick(float dt_s)
    {
        onTick(dt_s);
    }

    void Actor::setTickable(bool enable)
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

    Actor::Actor()
        : inLevel_()
        , name_()
        , componentMap_()
        , conceptComponents_()
        , rootTransform_()
        , tickable_(false)
        , isActive_(false)
        , tickingProcess_()
    {
    }
}