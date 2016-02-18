#ifndef _KILLME_PHYSICS_H_
#define _KILLME_PHYSICS_H_

#include <memory>

namespace killme
{
    class RigidBody;

    struct Physics
    {
        static void startup();
        static void shutdown();
        static void addRigidBody(const std::shared_ptr<RigidBody>& body);
        static void removeRigidBody(const std::shared_ptr<RigidBody>& body);
        static void tickWorld(float dt);
        static void debugDrawWorld(bool debug);
    };
}

#endif