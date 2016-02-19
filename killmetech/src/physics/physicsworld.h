#ifndef _KILLME_PHYSICSWORLD_H_
#define _KILLME_PHYSICSWORLD_H_

#include <BulletCollision/CollisionDispatch/btCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDispatcher.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletDynamics/ConstraintSolver/btConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <LinearMath/btIDebugDraw.h>
#include <memory>
#include <unordered_set>

namespace killme
{
    class CollisionShape;
    class RigidBody;

    /** The physics world */
    /// TODO: DEBUG
    class PhysicsWorld
    {
    private:
        std::unique_ptr<btCollisionConfiguration> config_;
        std::unique_ptr<btDispatcher> dispather_;
        std::unique_ptr<btBroadphaseInterface> broadphase_;
        std::unique_ptr<btConstraintSolver> solver_;
        std::unique_ptr<btDynamicsWorld> world_;

        std::unordered_set<std::shared_ptr<RigidBody>> rigidBodies_;

        std::unique_ptr<btIDebugDraw> debugDrawer_;

    public:
        /** Constructs */
        PhysicsWorld();

        /** Destructs */
        ~PhysicsWorld();

        /** Starts debug draw */
        void debugDrawWorld(bool debug);

        /** Adds the rigid body into the world */
        void addRigidBody(const std::shared_ptr<RigidBody>& body);

        /** Adds the rigid body from the world */
        void removeRigidBody(const std::shared_ptr<RigidBody>& body);

        /** Advance world time */
        void tick(float dt_s);
    };
}

#endif