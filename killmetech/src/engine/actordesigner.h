#ifndef _KILLME_ACTORDESIGNER_H_
#define _KILLME_ACTORDESIGNER_H_

#include <memory>

namespace killme
{
    class Actor;

    class ActorDesigner
    {
    private:
        std::weak_ptr<Actor> actor_;

    public:
        virtual ~ActorDesigner() = default;

        void setActor(const std::weak_ptr<Actor>& actor) { actor_ = actor; }
        std::shared_ptr<Actor> lockActor() { return actor_.lock(); }

        virtual void onSpawned() {}
        virtual void onKilled() {}
        virtual void onTicked() {}
    };
}

#endif
