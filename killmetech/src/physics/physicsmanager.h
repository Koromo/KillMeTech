#ifndef _KILLME_PHYSICSMANAGER_H_
#define _KILLME_PHYSICSMANAGER_H_

#include <BulletCollision/CollisionDispatch/btCollisionConfiguration.h>
#include <BulletCollision/BroadphaseCollision/btDispatcher.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <BulletDynamics/ConstraintSolver/btConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <LinearMath/btIDebugDraw.h>
#include <memory>
#include <vector>

namespace killme
{
    class CollisionShape;
    class RigidBody;

    /** The physics manager */
    class PhysicsManager
    {
    private:
        std::unique_ptr<btCollisionConfiguration> config_;
        std::unique_ptr<btDispatcher> dispather_;
        std::unique_ptr<btBroadphaseInterface> broadphase_;
        std::unique_ptr<btConstraintSolver> solver_;
        std::unique_ptr<btDynamicsWorld> world_;
        std::vector<std::shared_ptr<RigidBody>> rigidBodies_;

        std::unique_ptr<btIDebugDraw> debugDrawer_;

    public:
        /** Initializes */
        void startup();

        /** Finalizes */
        void shutdown();

        /** Starts debug draw */
        void debugDrawMode();

        /** Creates the rigid body */
        std::shared_ptr<RigidBody> createRigidBody(const std::shared_ptr<CollisionShape> shape, float mass);

        /** Advance world time */
        void stepWorld(float dt);
    };

    extern PhysicsManager physicsManager;
}

#endif