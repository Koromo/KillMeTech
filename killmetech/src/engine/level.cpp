#include "level.h"
#include "actor.h"
#include "components/actorcomponent.h"
#include "audiosystem.h"
#include "graphicssystem.h"
#include "../processes/process.h"
#include "../physics/physicsworld.h"
#include "../audio/audioworld.h"
#include "../scene/scene.h"
#include <cassert>

namespace killme
{
    std::string Level::uniqueActorName()
    {
        return "_Actor_" + std::to_string(actorCounter_());
    }

    void Level::addActor(const std::string& name, const std::shared_ptr<Actor>& actor)
    {
        const auto check = actors_.emplace(name, actor);
        assert(check.second && ("Conflict actor name \'" + name + "\'.").c_str());

        actor->setOwnerLevel(this, name);
        actor->onSpawn();
        actor->activate();
    }

    void Level::killActor(const std::string& name)
    {
        const auto it = actors_.find(name);
        if (it != std::cend(actors_))
        {
            it->second->onKill();
            it->second->deactivate();
            it->second->setOwnerLevel(nullptr, "");
            actors_.erase(it);
        }
    }

    std::shared_ptr<Actor> Level::findActor(const std::string& name)
    {
        const auto it = actors_.find(name);
        if (it == std::cend(actors_))
        {
            return nullptr;
        }
        return it->second;
    }
    
    AudioWorld& Level::getAudioWorld()
    {
        return *audioWorld_;
    }

    PhysicsWorld& Level::getPhysicsWorld()
    {
        return *physicsWorld_;
    }

    Scene& Level::getGraphicsWorld()
    {
        return *graphicsWorld_;
    }

    /** begin this level */
    void Level::beginLevel()
    {
        physicsWorld_ = std::make_unique<PhysicsWorld>();
        audioWorld_ = std::make_unique<AudioWorld>(audioSystem.getDeviceDetails());
        graphicsWorld_ = std::make_unique<Scene>(graphicsSystem.getRenderSystem());
        onBeginLevel();
    }

    /** End this level */
    void Level::endLevel()
    {
        onEndLevel();
        auto it = std::cbegin(actors_);
        while (it != std::cend(actors_))
        {
            killme::killActor(*this, it->first);
            it = std::cbegin(actors_);
        }
        graphicsWorld_.reset();
        audioWorld_.reset();
        physicsWorld_.reset();
    }

    void Level::beginFrame()
    {
        onBeginFrameComponents_.update();
    }

    void Level::tickLevel(float dt_s)
    {
        onTickLevel(dt_s);
        tickingActors_.update(dt_s);
        tickingComponents_.update(dt_s);
        physicsWorld_->stepSimulation(dt_s);
        audioWorld_->simulate();
    }

    void Level::renderLevel(const FrameResource& frame)
    {
        graphicsWorld_->renderScene(frame);
    }

    Process Level::registerTickingActor(Actor& actor)
    {
        return tickingActors_.startProcess([&](float dt_s) { actor.onTick(dt_s); });
    }

    Process Level::registerTickingComponent(ActorComponent& component)
    {
        return tickingComponents_.startProcess([&](float dt_s) { component.onTick(dt_s); });
    }

    Process Level::registerOnBeginFrameComponent(ActorComponent& component)
    {
        return onBeginFrameComponents_.startProcess([&] { component.onBeginFrame(); });
    }

    void killActor(Level& level, const std::string& name)
    {
        level.killActor(name);
    }
}