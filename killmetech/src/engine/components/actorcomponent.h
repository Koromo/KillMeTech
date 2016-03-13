#ifndef _KILLME_ACTORCOMPONENT_H_
#define _KILLME_ACTORCOMPONENT_H_

#include "../eventdef.h"
#include "../actor.h"
#include "../level.h"
#include "../../processes/process.h"
#include "../../core/utility.h"
#include <memory>

/** When you define a component, you have to use this macro */
#define KILLME_COMPONENT_DEFINE_BEGIN(Type) \
    public: \
    virtual TypeNumber getComponentType() const { return typeNumber<Type>(); } \
    KILLME_EVENT_HOOKS_BEGIN

#define KILLME_COMPONENT_DEFINE_END KILLME_EVENT_HOOKS_END

namespace killme
{
    /** The interface of actor components. Components defines a function to an actor. */
    class ActorComponent
    {
        KILLME_COMPONENT_DEFINE_BEGIN(ActorComponent)
        KILLME_COMPONENT_DEFINE_END

    private:
        Actor* owner_;
        bool isActive_;
        bool tickable_;
        Process tickingProcess_;

    public:
        /** For drived classes */
        virtual ~ActorComponent() = default;

        /** Set the owner */
        void setOwner(Actor* owner);

        /** Return the owner */
        Actor& getOwnerActor();
        Level& getOwnerLevel();

        /** Whether owner is exists or not */
        bool hasOwner() const;

        /** On actor activated */
        void activate();

        /** On actor deactivated */
        void deactivate();

        /** Return whether this is active */
        bool isActive() const;

        /** Called every frame */
        void tick(float dt_s);

        /** If set to true, this component ticked every frame */
        void setTickable(bool enable);

    protected:
        /** Construct */
        ActorComponent();

    private:
        /** Called on activate */
        virtual void onActivate() {}

        /** Called on deactivate */
        virtual void onDeactivate() {}

        /** Called on tick */
        virtual void onTick(float dt_s) {}
    };
}

#endif