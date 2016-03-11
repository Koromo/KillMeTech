#ifndef _KILLME_LEBEL_H_
#define _KILLME_LEBEL_H_

#include "eventdef.h"
#include "../processes/processscheduler.h"
#include "../events/eventdispatcher.h"
#include "../core/utility.h"
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>
#include <cassert>

/** When you define a level, you have to use this macro */
#define KILLME_LEVEL_DEFINE_BEGIN KILLME_EVENT_HOOKS_BEGIN
#define KILLME_LEVEL_DEFINE_END KILLME_EVENT_HOOKS_END

namespace killme
{
    class Process;
    class Actor;
    class ActorComponent;
    class AudioWorld;
    class PhysicsWorld;
    class Scene;
    struct FrameResource;

    /** Level */
    class Level : public EventDispatcher
    {
        KILLME_LEVEL_DEFINE_BEGIN
        KILLME_LEVEL_DEFINE_END

    private:
        std::unordered_map<std::string, std::shared_ptr<Actor>> actors_;
        UniqueCounter<size_t> actorCounter_;

        std::unique_ptr<AudioWorld> audioWorld_;
        std::unique_ptr<PhysicsWorld> physicsWorld_;
        std::unique_ptr<Scene> graphicsWorld_;

        ProcessScheduler<float> tickingActors_;
        ProcessScheduler<float> tickingComponents_;

    public:
        /** Destruct */
        virtual ~Level();

        /** Add an actor to this level */
        template <class T>
        std::shared_ptr<T> spawnActor(const std::shared_ptr<T>& actor, const std::string& name = "", bool activate = true)
        {
            const auto actorName = name.empty() ? "_Actor_" + std::to_string(actorCounter_()) : name;
            const auto check = actors_.emplace(actorName, actor);
            assert(check.second && ("Conflict actor name \'" + actorName + "\'.").c_str());

            actor->spawned(this, actorName);
            if (activate)
            {
                actor->activate();
            }

            return actor;
        }

        /** Remove an actor from this level */
        void despawnActor(const std::string& name);

        /** Search an actor */
        std::shared_ptr<Actor> findActor(const std::string& name);

        /** Return the audio world of this level */
        AudioWorld& getAudioWorld();

        /** Return the physics world of this level */
        PhysicsWorld& getPhysicsWorld();

        /** Return the graphics world of this level */
        Scene& getGraphicsWorld();

        /** Begin this level */
        void begin();

        /** End this level */
        void end();

        /** Advance time of this level */
        void tick(float dt_s);

        /** Draw current level */
        void draw(const FrameResource& frame);

        /** Register an actor for ticking */
        Process registerTicking(Actor& actor);

        /** Register a component for ticking */
        Process registerTicking(ActorComponent& component);

    protected:
        /** Construct */
        Level();

        /** Called on level begin */
        virtual void onBegin() {}

        /** Called on end level */
        virtual void onEnd() {}

        /** Called on tick level */
        virtual void onTick(float dt_s) {}
    };
}

#endif