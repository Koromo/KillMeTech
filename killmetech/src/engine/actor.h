#ifndef _KILLME_ACTOR_H_
#define _KILLME_ACTOR_H_

#include "eventdef.h"
#include "level.h"
#include "../events/eventdispatcher.h"
#include "../processes/process.h"
#include "../core/utility.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <type_traits>
#include <cassert>

/** When you define an actor, you have to use this macro */
#define KILLME_ACTOR_DEFINE_BEGIN KILLME_EVENT_HOOKS_BEGIN
#define KILLME_ACTOR_DEFINE_END KILLME_EVENT_HOOKS_END

namespace killme
{
    class ActorComponent;
    class TransformComponent;

    /** The actor is an element of world */
    /// NOTE: If you need any components, you have to attach all needed components in constructor
    class Actor : public EventDispatcher
    {
        KILLME_ACTOR_DEFINE_BEGIN
        KILLME_ACTOR_DEFINE_END

    private:
        Level* inLevel_;
        std::string name_;
        std::unordered_multimap<TypeNumber, std::shared_ptr<ActorComponent>> componentMap_;
        std::vector<std::shared_ptr<ActorComponent>> conceptComponents_;
        std::shared_ptr<TransformComponent> rootTransform_;
        bool tickable_;
        bool isActive_;
        Process tickingProcess_;

    public:
        /** For drived classes */
        virtual ~Actor() = default;

        /** Called on spawned */
        void spawned(Level* inLevel, const std::string& name);

        /** Called on despawned */
        void despawned();

        /** Return owner level */
        Level& getOwnerLevel();

        /** Return the name */
        std::string getName() const;

        /** Activate this actor */
        void activate();

        /** Deactivate this actor */
        void deactivate();

        // Attach a concept component to this actor
        void attachConceptComponentImpl(const std::shared_ptr<ActorComponent>& component);

        // Attach transform components to this actor
        void attachTransformedComponentImpl(const std::shared_ptr<TransformComponent>& component);

        /** Attach a concept component to this actor */
        /// NOTE: If you want to attach transform components, you need use attachRootTransform().
        template <class T>
        std::shared_ptr<T> attachConceptComponent(const std::shared_ptr<T>& component)
        {
            //static_assert(!std::is_base_of<TransformComponent, T>::value, "You can not attach transform components by attachConceptComponent().");
            attachConceptComponentImpl(component);
            conceptComponents_.emplace_back(component);
            return component;
        }

        /** Attach the root transform to this actor */
        /// NOTE: If you need transform components, you need to attach root transform only.
        ///       Child components of root transform are attached automatically.
        template <class T>
        std::shared_ptr<T> attachRootTransform(const std::shared_ptr<T>& root)
        {
            //static_assert(std::is_base_of<TransformComponent, T>::value, "You can not attach oncept components by attachRootTransform().");
            assert(!rootTransform_ && "This actor is already has root transform.");
            attachTransformedComponentImpl(root);
            rootTransform_ = root;

            return root;
        }

        /** Return the root transform */
        std::shared_ptr<TransformComponent> getRootTransform();

        /** Search components */
        auto getComponents(TypeNumber type)
            -> decltype(iteratorRange(componentMap_.equal_range(type).first, componentMap_.equal_range(type).second))
        {
            const auto range = componentMap_.equal_range(type);
            return iteratorRange(range.first, range.second);
        }

        /** Called every frame */
        void tick(float dt_s);

        /** If set to true, this actor ticked every frame */
        void setTickable(bool enable);

    protected:
        /** Construct */
        Actor();

        /** Called on spawned */
        virtual void onSpawn() {}

        /** Called on despawned */
        virtual void onDespawn() {}

        /** Called on activate */
        virtual void onActivate() {}

        /** Called on deactivate */
        virtual void onDeactivate() {}

        /** Called on tick */
        virtual void onTick(float dt_s) {}
    };

    /** Search an component */
    template <class Component>
    std::shared_ptr<Component> findComponent(Actor& actor)
    {
        const auto range = actor.getComponents(typeNumber<Component>());
        const auto front = std::cbegin(range);
        if (front == std::cend(range))
        {
            return nullptr;
        }
        return std::static_pointer_cast<Component>(front->second);
    }
}

#endif