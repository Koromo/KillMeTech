#ifndef _KILLME_ACTORCOMPONENT_H_
#define _KILLME_ACTORCOMPONENT_H_

#include "../../processes/process.h"
#include "../../core/utility.h"
#include <memory>
#include <vector>

namespace killme
{
    class Actor;
    class Level;

#define KILLME_COMPONENT_DEFINE(Type) \
public: \
virtual TypeNumber getComponentType() const { return typeNumber<Type>(); } \
private:

    /** The interface of actor components. Components defines function of an actor. */
    class ActorComponent
    {
        KILLME_COMPONENT_DEFINE(ActorComponent)

    private:
        Actor* owner_;
        bool isActive_;
        bool enableTicking_;
        bool enableBeginFrame_;
        std::vector<Process> processes_;

    protected:
        /** Construct */
        ActorComponent();

        /** For drived classes */
        virtual ~ActorComponent() = default;

    public:
        /** Owner accessor */
        void setOwnerActor(Actor* owner);
        Actor& getOwnerActor();
        Level& getOwnerLevel();

        /** Activate component */
        void activate();

        /** Deactivate component */
        void deactivate();

        /** Return whether this is active */
        bool isActive() const;

        /** Called on activate */
        virtual void onActivate() {}

        /** Called on deactivate */
        virtual void onDeactivate() {}

        /** Called every frame */
        virtual void onTick(float dt_s) {}

        /** Called on begin frame */
        virtual void onBeginFrame() {}

        /** If you call this, onTick() is called every frame */
        /// NOTE: You need call this until end of onActivate()
        void enableTicking();

        /** If you call this, onBeginFrame() is called every frame begin */
        /// NOTE: You need call this until end of onActivate()
        void enableBeginFrame();
    };
}

#endif