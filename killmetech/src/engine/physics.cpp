#include "physics.h"
#include "../physics/physicsworld.h"

namespace killme
{
    namespace
    {
        std::unique_ptr<PhysicsWorld> world;
    }
    
    void Physics::startup()
    {
        world = std::make_unique<PhysicsWorld>();
    }

    void Physics::shutdown()
    {
        world.reset();
    }

    void Physics::addRigidBody(const std::shared_ptr<RigidBody>& body)
    {
        world->addRigidBody(body);
    }

    void Physics::removeRigidBody(const std::shared_ptr<RigidBody>& body)
    {
        world->removeRigidBody(body);
    }

    void Physics::tickWorld(float dt)
    {
        world->tick(dt);
    }

    void Physics::debugDrawWorld(bool debug)
    {
        world->debugDrawWorld(debug);
    }
}