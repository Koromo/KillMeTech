#include "level.h"
#include "actor.h"
#include "components/actorcomponent.h"
#include "audiosystem.h"
#include "graphicssystem.h"
#include "../processes/process.h"
#include "../physics/physicsworld.h"
#include "../audio/audioworld.h"
#include "../scene/scene.h"

namespace killme
{
    Level::~Level()
    {
        auto it = std::cbegin(actors_);
        while (it != std::cend(actors_))
        {
            despawnActor(it->first);
            it = std::cbegin(actors_);
        }
    }

    void Level::despawnActor(const std::string& name)
    {
        const auto it = actors_.find(name);
        if (it != std::cend(actors_))
        {
            it->second->deactivate();
            it->second->despawned();
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

    void Level::begin()
    {
        KILLME_CONNECT_EVENT_HOOKS();
        onBegin();
    }

    void Level::end()
    {
        onEnd();
        KILLME_DISCONNECT_EVENT_HOOKS();
    }

    void Level::tick(float dt_s)
    {
        onTick(dt_s);
        tickingActors_.update(dt_s);
        tickingComponents_.update(dt_s);
        physicsWorld_->stepSimulation(dt_s);
        audioWorld_->simulate();
    }

    void Level::draw(const FrameResource& frame)
    {
        graphicsWorld_->renderScene(frame);
    }

    Process Level::registerTicking(Actor& actor)
    {
        return tickingActors_.startProcess([&](float dt_s) { actor.tick(dt_s); });
    }

    Process Level::registerTicking(ActorComponent& component)
    {
        return tickingComponents_.startProcess([&](float dt_s) { component.tick(dt_s); });
    }

    Level::Level()
        : actors_()
        , actorCounter_()
        , audioWorld_()
        , physicsWorld_()
        , graphicsWorld_()
        , tickingActors_()
        , tickingComponents_()
    {
        physicsWorld_ = std::make_unique<PhysicsWorld>();
        audioWorld_ = std::make_unique<AudioWorld>(audioSystem.getDeviceDetails());
        graphicsWorld_ = std::make_unique<Scene>(graphicsSystem.getRenderSystem());
    }
}