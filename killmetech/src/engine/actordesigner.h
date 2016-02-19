#ifndef _KILLME_ACTORDESIGNER_H_
#define _KILLME_ACTORDESIGNER_H_

#include <memory>

namespace killme
{
    class Actor;

    /** Script for an actor */
    class ActorDesigner
    {
    private:
        std::weak_ptr<Actor> actor_;

    public:
        /** For drived classes */
        virtual ~ActorDesigner() = default;

        /** Actor accessor */
        void setActor(const std::weak_ptr<Actor>& actor) { actor_ = actor; }
        std::shared_ptr<Actor> lockActor() { return actor_.lock(); }

        /** Called on actor spawned */
        virtual void onSpawned() {}

        /** Called on actor killed */
        virtual void onKilled() {}

        /** Called on actor ticked */
        virtual void onTicked(float dt_s) {}
    };
}

#endif
