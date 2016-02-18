#ifndef _KILLME_PHYSICS_H_
#define _KILLME_PHYSICS_H_

#include <memory>

namespace killme
{
    class RigidBody;

    /** Physics engine subsystem */
    struct Physics
    {
        /** Startup physics */
        static void startup();

        /** Shutdown physics */
        static void shutdown();

        /** Add rigid body into the physics world */
        static void addRigidBody(const std::shared_ptr<RigidBody>& body);

        /** Remove rigid body from the physics world */
        static void removeRigidBody(const std::shared_ptr<RigidBody>& body);

        /** Step simulation */
        static void tickWorld(float dt);

        /** Whether draw physics world for debug (default false) */
        static void debugDrawWorld(bool debug);
    };
}

#endif