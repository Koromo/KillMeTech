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
    class Vector3;
    class Color;

    /** Debug drawer */
    class PhysicsDebugDrawer : public btIDebugDraw
    {
        virtual void line(const Vector3& from, const Vector3& to, const Color& color) = 0;

        void drawLine(const btVector3& from, const btVector3& to_, const btVector3& color);
        void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&);
        void reportErrorWarning(const char*);
        void draw3dText(const btVector3&, const char*);
        void setDebugMode(int);
        int getDebugMode() const;
    };

    /** Physics world */
    class PhysicsWorld
    {
    private:
        std::unique_ptr<btCollisionConfiguration> config_;
        std::unique_ptr<btDispatcher> dispather_;
        std::unique_ptr<btBroadphaseInterface> broadphase_;
        std::unique_ptr<btConstraintSolver> solver_;
        std::unique_ptr<btDynamicsWorld> world_;

        std::unordered_set<std::shared_ptr<RigidBody>> rigidBodies_;

        std::shared_ptr<btIDebugDraw> debugDrawer_;

    public:
        /** Construct */
        PhysicsWorld();

        /** Destruct */
        ~PhysicsWorld();

        /** Start draw the world for debug */
        void debugDraw(const std::shared_ptr<PhysicsDebugDrawer>& drawer);

        /** Add a rigid body into the world */
        void addRigidBody(const std::shared_ptr<RigidBody>& body);

        /** Remove a rigid body from the world */
        void removeRigidBody(const std::shared_ptr<RigidBody>& body);

        /** Advance world time */
        void stepSimulation(float dt_s);
    };
}

#endif