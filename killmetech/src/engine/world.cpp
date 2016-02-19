#include "world.h"
#include "actordesigner.h"

namespace killme
{
    World::~World()
    {
        auto it = std::cbegin(actors_);
        while (it != std::cend(actors_))
        {
            killActor(it->first);
            it = std::cbegin(actors_);
        }
    }

    std::shared_ptr<Actor> World::spawnActor(const std::string& name)
    {
        const auto actor = std::make_shared<Actor>(name);

        const auto check = actors_.emplace(name, actor); // Insert
        assert(check.second && "Conflicts actor name.");

        actor->setOwnerWorld(shared_from_this()); // Set owner
        actor->onSpawned(); // Spawned
        return actor;
    }

    void World::killActor(const std::string& name)
    {
        const auto it = actors_.find(name);
        if (it != std::cend(actors_))
        {
            it->second->onKilled(); // Killed
            it->second->setOwnerWorld(std::shared_ptr<World>()); // Null owner
            actors_.erase(it); // Erase
        }
    }

    std::shared_ptr<Actor> World::findActor(const std::string& name)
    {
        const auto it = actors_.find(name);
        if (it == std::cend(actors_))
        {
            return nullptr;
        }
        return it->second;
    }

    void World::tickActors(float dt_s)
    {
        for (const auto& actor : actors_)
        {
            actor.second->tick(dt_s);
        }
    }
}