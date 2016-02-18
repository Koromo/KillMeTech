#ifndef _KILLME_LEVELDESIGNER_H_
#define _KILLME_LEVELDESIGNER_H_

#include "world.h"
#include "components/transformcomponent.h"
#include "components/cameracomponent.h"
#include "components/audiolistenercomponent.h"
#include "actor.h"
#include <memory>

namespace killme
{
    class World;

    /** Script for a level */
    class LevelDesigner
    {
    private:
        std::shared_ptr<World> world_;

    public:
        /** For drived classes */
        virtual ~LevelDesigner() = default;

        /** Game world accessor */
        void setWorld(const std::shared_ptr<World>& world) { world_ = world; }
        std::shared_ptr<World> getWorld() const { return world_; };

        /** Build default world */
        void defaultBuild()
        {
            const auto actor = world_->spawnActor("Actor");
            actor->placement<TransformComponent>();
            actor->attachComponent<CameraComponent>("Camera");
            actor->attachComponent<AudioListenerComponent>("Listener");
        }

        /** Build game world */
        virtual void build() {}

        /** Tick game world */
        virtual void tick() {}
    };
}

#endif