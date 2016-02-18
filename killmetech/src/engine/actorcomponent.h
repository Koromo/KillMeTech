#ifndef _KILLME_ACTORCOMPONENT_H_
#define _KILLME_ACTORCOMPONENT_H_

#include <memory>

namespace killme
{
    class Actor;

    /** The interface of actor components. Components defines behaviour of an actor. */
    class ActorComponent
    {
    private:
        std::weak_ptr<Actor> owner_;

    public:
        virtual ~ActorComponent() = default;

        virtual void onAttached() {}
        virtual void onDettached() {}

        void setOwner(const std::weak_ptr<Actor>& owner) { owner_ = owner; }
        std::shared_ptr<Actor> lockOwner() { return owner_.lock(); }
    };
}

#endif