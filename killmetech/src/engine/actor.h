#ifndef _KILLME_ACTOR_H_
#define _KILLME_ACTOR_H_

#include "../processes/process.h"
#include "../core/utility.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <type_traits>
#include <cassert>

namespace killme
{
    class Level;
    class ActorComponent;
    class TransformComponent;

    /** The actor is an element of world */
    class Actor
    {
    private:
        Level* inLevel_;
        std::string name_;
        std::unordered_multimap<TypeNumber, std::shared_ptr<ActorComponent>> componentMap_;
        std::vector<std::shared_ptr<ActorComponent>> conceptComponents_;
        std::shared_ptr<TransformComponent> rootTransform_;
        bool enableTicking_;
        bool isActive_;
        Process tickProcess_;

    public:
        /** Construct */
        Actor();

        /** For drived classes */
        virtual ~Actor() = default;

        /** Set owner level and name */
        void setOwnerLevel(Level* inLevel, const std::string& name);

        /** Return owner level */
        Level& getOwnerLevel();

        /** Return the name */
        std::string getName() const;

        /** Activate this actor */
        void activate();

        /** Deactivate this actor */
        void deactivate();

        /** Kill this actor */
        void kill();

        /** Attach a component to an actor */
        /// NOTE: You need attach all components until end of onSpawn()
        /// NOTE: If you want to attach transformed components, you no need to call this method. See a note of attachRootTransform().
        template <class T>
        std::shared_ptr<T> attachComponent(const std::shared_ptr<T>& component)
        {
            static_assert(!std::is_base_of<TransformComponent, T>::value, "You can not attach transformed component by this method.");
            conceptComponents_.emplace_back(component);
            return component;
        }

        /** Attach the root transform to an actor */
        /// NOTE: If you need transform components, you need to attach root transform only.
        template <class T>
        std::shared_ptr<T> attachRootTransform(const std::shared_ptr<T>& root)
        {
            static_assert(std::is_base_of<TransformComponent, T>::value, "You can not attach non transformed component by this method.");
            assert(!rootTransform_ && "This actor is already has root transform.");
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

        /** Called on just before spawned */
        virtual void onSpawn() {}

        /** Called on just before killed */
        virtual void onKill() {}

        /** Called every frame */
        virtual void onTick(float dt_s) {}

        /** If you call this function before end of onSpawn(), this actor is never called onTick() function */
        void disableTicking();
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