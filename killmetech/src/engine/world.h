#ifndef _KILLME_WORLD_H_
#define _KILLME_WORLD_H_

#include "actor.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>
#include <cassert>

namespace killme
{
    /** The game world */
    class World : public std::enable_shared_from_this<World>
    {
    private:
        std::unordered_map<std::string, std::shared_ptr<Actor>> actors_;

    public:
        /** Destruct */
        ~World();

        /** Spawn an actor */
        std::shared_ptr<Actor> spawnActor(const std::string& name);

        /** Spawn an actor with designer */
        template <class Designer, class... Args>
        std::shared_ptr<Actor> spawnDesignedActor(const std::string& name, Args&&... args)
        {
            const auto actor = std::make_shared<Actor>(name);
            const auto designer = std::make_shared<Designer>(std::forward<Args>(args)...);

            const auto check = actors_.emplace(name, actor); // Insert
            assert(check.second && "Conflicts actor name.");

            actor->setOwnerWorld(shared_from_this()); // Set owner
            actor->setDesigner(designer);
            actor->onSpawned(); // Spawned

            return actor;
        }

        /** Kill an actor */
        void killActor(const std::string& name);

        /** Search an actor */
        std::shared_ptr<Actor> findActor(const std::string& name);

        /** Tick actors */
        void tickActors(float dt_s);
    };
}

#endif