#include "actor.h"
#include "transformcomponent.h"
#include "actordesigner.h"
#include "world.h"

namespace killme
{
    const std::string NAME_ROOT_TRANSFORM = "_RootTransform";

    Actor::Actor(const std::string& name)
        : ownerWorld_()
        , name_(name)
        , rootTransform_()
        , componentMap_()
        , designer_()
    {
    }

    void Actor::setOwnerWorld(const std::weak_ptr<World>& ownerWorld)
    {
        ownerWorld_ = ownerWorld;
    }

    void Actor::onSpawned()
    {
        if (designer_)
        {
            designer_->onSpawned(); // After initializes
        }
    }

    void Actor::onKilled()
    {
        if (designer_)
        {
            designer_->onKilled(); // Before finalizes
        }

        while (componentMap_.size() > static_cast<size_t>(rootTransform_ ? 1 : 0))
        {
            auto it = std::cbegin(componentMap_);
            if (it->first == NAME_ROOT_TRANSFORM)
            {
                ++it;
            }
            dettachComponent(it->first);
        }

        if (rootTransform_)
        {
            rootTransform_->onDettached(); // Dettached
            rootTransform_->setOwner(std::shared_ptr<Actor>()); // Null owner
            rootTransform_.reset(); // Erase
            componentMap_.clear();
        }
    }

    void Actor::kill()
    {
        if (const auto w = ownerWorld_.lock())
        {
            w->killActor(name_);
        }
    }

    void Actor::setDesigner(const std::shared_ptr<ActorDesigner>& designer)
    {
        designer_ = designer;
        designer_->setActor(shared_from_this());
    }

    void Actor::dettachComponent(const std::string& name)
    {
        assert(name != NAME_ROOT_TRANSFORM && "Root transform can not dettach.");
        const auto it = componentMap_.find(name);
        if (it != std::cend(componentMap_))
        {
            it->second->onDettached(); // Dettached
            it->second->setOwner(std::shared_ptr<Actor>()); // Null owner
            componentMap_.erase(it); // Erase
        }
    }

    void Actor::tick()
    {
        if (designer_)
        {
            designer_->onTicked();
        }
    }

    void Actor::createTransformTree(const std::shared_ptr<TransformComponent>& c)
    {
        assert(rootTransform_ &&
            "You can not attach any transformed components becouse the actor is not placemented into the game world.");
        rootTransform_->addChild(c);
    }
}