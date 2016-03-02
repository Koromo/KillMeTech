#ifndef _KILLME_ACTOR_H_
#define _KILLME_ACTOR_H_

#include "components/actorcomponent.h"
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>
#include <type_traits>
#include <cassert>

namespace killme
{
    class World;
    class ActorDesigner;
    class TransformComponent;

    extern const std::string NAME_ROOT_TRANSFORM;

    /** The actor is an element of level */
    class Actor : public std::enable_shared_from_this<Actor>
    {
    private:
        std::weak_ptr<World> ownerWorld_;
        std::string name_;
        std::shared_ptr<TransformComponent> rootTransform_;
        std::unordered_map<std::string, std::shared_ptr<ActorComponent>> componentMap_;
        std::shared_ptr<ActorDesigner> designer_;

    public:
        /** Construct */
        explicit Actor(const std::string& name);

        /** Set owner world */
        void setOwnerWorld(const std::weak_ptr<World>& ownerWorld);

        /** Called on spawned */
        void onSpawned();

        /** Called on killed */
        void onKilled();

        /** Kill this actor */
        void kill();

        /** Set designer */
        void setDesigner(const std::shared_ptr<ActorDesigner>& designer);

        /** Placement into the game world */
        template <class T, class... Args>
        std::shared_ptr<T> placement(Args&&... args)
        {
            assert(!rootTransform_ && "The actor is already placemented into the world.");

            const auto trans = this->template attachComponentImpl<T>(NAME_ROOT_TRANSFORM, std::forward<Args>(args)...);
            rootTransform_ = trans;
            rootTransform_->onAttached();

            return trans;
        }

        /** Return root transform */
        template <class T = TransformComponent>
        std::shared_ptr<T> getRootTransform()
        {
            assert(rootTransform_ && "The actor is not placemented into the world.");

            const auto trans = std::dynamic_pointer_cast<T>(rootTransform_);
            assert(trans && "Conflict component type.");

            return trans;
        }

        /** Attach the component */
        template <class T, class... Args>
        std::shared_ptr<T> attachComponent(const std::string& name, Args&&... args)
        {
            assert(name != NAME_ROOT_TRANSFORM && "The name of component need to different to root transform.");

            const auto c = this->template attachComponentImpl<T>(name, std::forward<Args>(args)...);

            if (std::is_base_of<TransformComponent, T>::value)
            {
                createTransformTree(c); // Tree
            }
            c->onAttached(); // Attached

            return c;
        }

        /** Dettache component */
        void dettachComponent(const std::string& name);

        /** Search component */
        template <class T>
        std::shared_ptr<T> findComponent(const std::string& name)
        {
            const auto it = componentMap_.find(name);
            if (it == std::cend(componentMap_))
            {
                return nullptr;
            }
            return std::dynamic_pointer_cast<T>(it->second);
        }

        /** Tick actor */
        void tick(float dt_s);

    private:
        template <class T, class... Args>
        std::shared_ptr<T> attachComponentImpl(const std::string& name, Args&&... args)
        {
            assert(!ownerWorld_.expired() && "The negavite actor can not attach any components.");

            const auto c = std::make_shared<T>(std::forward<Args>(args)...);
            const auto check = componentMap_.emplace(name, c); // Insert
            assert(check.second && "Conflicts component names.");

            c->setOwner(shared_from_this()); // Set owner

            return c;
        }

        void createTransformTree(const std::shared_ptr<TransformComponent>& c);
    };
}

#endif