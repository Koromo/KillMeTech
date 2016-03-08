#ifndef _KILLME_LEBEL_H_
#define _KILLME_LEBEL_H_

#include "../processes/processscheduler.h"
#include "../events/eventdispatcher.h"
#include "../core/utility.h"
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>

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
    private:
        std::unordered_map<std::string, std::shared_ptr<Actor>> actors_;
        UniqueCounter<size_t> actorCounter_;

        std::unique_ptr<AudioWorld> audioWorld_;
        std::unique_ptr<PhysicsWorld> physicsWorld_;
        std::unique_ptr<Scene> graphicsWorld_;

        ProcessScheduler<float> tickingActors_;
        ProcessScheduler<float> tickingComponents_;
        ProcessScheduler<> onBeginFrameComponents_;

    public:
        /** For drived classes */
        virtual ~Level() = default;

        // For spawnActorWithName()
        std::string uniqueActorName();

        // ditto
        void addActor(const std::string& name, const std::shared_ptr<Actor>& actor);

        // For killme::killActor()
        void killActor(const std::string& name);

        /** Search an actor */
        std::shared_ptr<Actor> findActor(const std::string& name);

        /** Return the audio world of this level */
        AudioWorld& getAudioWorld();

        /** Return the physics world of this level */
        PhysicsWorld& getPhysicsWorld();

        /** Return the graphics world of this level */
        Scene& getGraphicsWorld();

        /** begin this level */
        void beginLevel();

        /** End this level */
        void endLevel();

        /** Begin frame */
        void beginFrame();

        /** Advance time of this level */
        void tickLevel(float dt_s);

        /** Draw current level */
        void renderLevel(const FrameResource& frame);

        /** Register an actor for ticking */
        Process registerTickingActor(Actor& actor);

        /** Register a component for ticking */
        Process registerTickingComponent(ActorComponent& component);

        /** Register a component for onBeginFrame() */
        Process registerOnBeginFrameComponent(ActorComponent& component);

        /** Called every frame */
        virtual void onTickLevel(float dt_s) {}

        /** Called on begin level */
        virtual void onBeginLevel() {}

        /** Called on end level */
        virtual void onEndLevel() {}
    };

    /** Spawn an actor in a level */
    template <class T, class... Args>
    std::shared_ptr<T> spawnActorWithName(Level& level, std::string& name, Args&&... args)
    {
        const auto actor = std::make_shared<T>(std::forward<Args>(args)...);
        level.addActor(name, actor);
        return actor;
    }

    /** ditto */
    template <class T, class... Args>
    std::shared_ptr<T> spawnActor(Level& level, Args&&... args)
    {
        return spawnActorWithName<T>(level, level.uniqueActorName(), std::forward<Args>(args)...);
    }

    /** Kill an actor in a level */
    void killActor(Level& level, const std::string& name);
}

#endif