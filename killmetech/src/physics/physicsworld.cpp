#include "physicsworld.h"
#include "collisionshape.h"
#include "rigidbody.h"
#include "bulletsupport.h"
#include "../scene/debugdrawmanager.h"
#include "../core/math/color.h"
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <algorithm>
#include <cassert>

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

    PhysicsWorld::PhysicsWorld()
        : config_()
        , dispather_()
        , broadphase_()
        , solver_()
        , world_()
        , rigidBodies_()
    {
        config_ = std::make_unique<btDefaultCollisionConfiguration>();
        dispather_ = std::make_unique<btCollisionDispatcher>(config_.get());
        broadphase_ = std::make_unique<btDbvtBroadphase>();
        solver_ = std::make_unique<btSequentialImpulseConstraintSolver>();
        world_ = std::make_unique<btDiscreteDynamicsWorld>(dispather_.get(), broadphase_.get(), solver_.get(), config_.get());
    }
    
    PhysicsWorld::~PhysicsWorld()
    {
        for (int i = world_->getNumCollisionObjects() - 1; i >= 0; --i)
        {
            const auto obj = world_->getCollisionObjectArray()[i];
            world_->removeCollisionObject(obj);
        }
        rigidBodies_.clear();
    }

    void PhysicsWorld::debugDrawWorld(bool debug)
    {
        if (debug)
        {
            debugDrawer_ = std::make_unique<DebugDrawer>();
            world_->setDebugDrawer(debugDrawer_.get());
        }
        else
        {
            world_->setDebugDrawer(nullptr);
            debugDrawer_.reset();
        }
    }

    void PhysicsWorld::addRigidBody(const std::shared_ptr<RigidBody>& body)
    {
        const auto check = rigidBodies_.emplace(body);
        assert(check.second && "Conflicts the rigid body.");

        world_->addRigidBody(body->getBtBody());
    }

    void PhysicsWorld::removeRigidBody(const std::shared_ptr<RigidBody>& body)
    {
        const auto n = rigidBodies_.erase(body);
        if (n > 0)
        {
            world_->removeRigidBody(body->getBtBody());
        }
    }

    void PhysicsWorld::tick(float dt_s)
    {
        const auto fixedTimeStep = 0.01666666754f;
        world_->stepSimulation(dt_s, static_cast<int>(dt_s / fixedTimeStep + 1.0001f), fixedTimeStep);
        if (debugDrawer_)
        {
            world_->debugDrawWorld();
        }
    }
}