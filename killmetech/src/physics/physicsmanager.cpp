#include "physicsmanager.h"
#include "collisionshape.h"
#include "rigidbody.h"
#include "bulletsupport.h"
#include "../scene/debugdrawmanager.h"
#include "../core/platform.h"
#include "../core/math/color.h"
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

namespace killme
{
    namespace
    {
        struct DebugDrawer : btIDebugDraw
        {
            void drawLine(const btVector3& from, const btVector3& to_, const btVector3& color)
            {
                const Color c = {color.x(), color.y(), color.z(), 1};
                debugDrawManager.line(to<Vector3>(from), to<Vector3>(to_), c);
            }

            void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) {}
            void reportErrorWarning(const char*) {}
            void draw3dText(const btVector3&, const char*) {}
            void setDebugMode(int) {}
            int getDebugMode() const { return btIDebugDraw::DBG_DrawWireframe; }
        };
    }

    PhysicsManager physicsManager;

    void PhysicsManager::startup()
    {
        config_ = std::make_unique<btDefaultCollisionConfiguration>();
        dispather_ = std::make_unique<btCollisionDispatcher>(config_.get());
        broadphase_ = std::make_unique<btDbvtBroadphase>();
        solver_ = std::make_unique<btSequentialImpulseConstraintSolver>();
        world_ = std::make_unique<btDiscreteDynamicsWorld>(dispather_.get(), broadphase_.get(), solver_.get(), config_.get());
    }

    void PhysicsManager::shutdown()
    {
        debugDrawer_.reset();

        for (int i = world_->getNumCollisionObjects() - 1; i >= 0; --i)
        {
            const auto obj = world_->getCollisionObjectArray()[i];
            world_->removeCollisionObject(obj);
        }
        rigidBodies_.clear();
        world_.reset();
        solver_.reset();
        broadphase_.reset();
        dispather_.reset();
        config_.reset();
    }

    void PhysicsManager::debugDrawMode()
    {
#ifdef KILLME_DEBUG
        debugDrawer_ = std::make_unique<DebugDrawer>();
        world_->setDebugDrawer(debugDrawer_.get());
#endif
    }

    std::shared_ptr<RigidBody> PhysicsManager::createRigidBody(const std::shared_ptr<CollisionShape> shape, float mass)
    {
        if (shape->getType() == ShapeType::static_)
        {
            mass = 0;
        }
        const auto body = std::make_shared<RigidBody>(shape, mass);
        world_->addRigidBody(body->getBtBody());
        rigidBodies_.push_back(body);
        return body;
    }


    void PhysicsManager::stepWorld(float dt)
    {
        world_->stepSimulation(dt);
        if (debugDrawer_)
        {
            world_->debugDrawWorld();
        }
    }
}