#ifndef _KILLME_LEVELDESIGNER_H_
#define _KILLME_LEVELDESIGNER_H_

#include "world.h"
#include "transformcomponent.h"
#include "cameracomponent.h"
#include "audiolistenercomponent.h"
#include "actor.h"
#include <memory>

namespace killme
{
    class World;

    class LevelDesigner
    {
    private:
        std::shared_ptr<World> world_;

    public:
        virtual ~LevelDesigner() = default;

        void setWorld(const std::shared_ptr<World>& world) { world_ = world; }
        std::shared_ptr<World> getWorld() const { return world_; };

        void defaultBuild()
        {
            const auto actor = world_->spawnActor("Actor");
            actor->placement<TransformComponent>();
            actor->attachComponent<CameraComponent>("Camera");
            actor->attachComponent<AudioListenerComponent>("Listener");
        }

        virtual void build() {}
        virtual void tick() {}
    };
}

#endif