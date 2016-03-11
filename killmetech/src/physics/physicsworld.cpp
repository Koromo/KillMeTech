#include "physicsworld.h"
#include "collisionshape.h"
#include "rigidbody.h"
#include "bulletsupport.h"
#include "../core/math/color.h"
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <algorithm>
#include <unordered_map>
#include <cassert>

namespace killme
{
    void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to_, const btVector3& color)
    {
        const Color c = { color.x(), color.y(), color.z(), 1 };
        line(to<Vector3>(from), to<Vector3>(to_), c);
    }

    void PhysicsDebugDrawer::drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) {}
    void PhysicsDebugDrawer::reportErrorWarning(const char*) {}
    void PhysicsDebugDrawer::draw3dText(const btVector3&, const char*) {}
    void PhysicsDebugDrawer::setDebugMode(int) {}
    int PhysicsDebugDrawer::getDebugMode() const { return btIDebugDraw::DBG_DrawWireframe; }

    namespace
    {
        void tickCallback(btDynamicsWorld* world, btScalar)
        {
            const auto cllidedObjects = static_cast<std::unordered_map<RigidBody*, std::unordered_set<RigidBody*>>*>(world->getWorldUserInfo());
            const auto numManifolds = world->getDispatcher()->getNumManifolds();
            for (int i = 0; i < numManifolds; i++)
            {
                btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
                auto objA = static_cast<RigidBody*>(contactManifold->getBody0()->getUserPointer());
                auto objB = static_cast<RigidBody*>(contactManifold->getBody0()->getUserPointer());
                if (objA < objB)
                {
                    cllidedObjects->operator[](objA).emplace(objB);
                }
                else
                {
                    cllidedObjects->operator[](objB).emplace(objA);
                }
            }
        }
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
        world_->setInternalTickCallback(tickCallback);
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

    void PhysicsWorld::debugDraw(const std::shared_ptr<PhysicsDebugDrawer>& drawer)
    {
        debugDrawer_ = drawer;
        world_->setDebugDrawer(debugDrawer_.get());
    }

    void PhysicsWorld::addRigidBody(const std::shared_ptr<RigidBody>& body)
    {
        const auto added = rigidBodies_.emplace(body);
        if (added.second)
        {
            world_->addRigidBody(body->getBtBody());
        }
    }

    void PhysicsWorld::removeRigidBody(const std::shared_ptr<RigidBody>& body)
    {
        const auto n = rigidBodies_.erase(body);
        if (n > 0)
        {
            world_->removeRigidBody(body->getBtBody());
        }
    }

    void PhysicsWorld::stepSimulation(float dt_s)
    {
        static const auto FIXED_TIME_STEP = 0.01666666754f;
        std::unordered_map<RigidBody*, std::unordered_set<RigidBody*>> collidedObjects;

        world_->setWorldUserInfo(&collidedObjects);
        world_->stepSimulation(dt_s, static_cast<int>(dt_s / FIXED_TIME_STEP + 1.0001f), FIXED_TIME_STEP);
        world_->setWorldUserInfo(nullptr);

        for (const auto& objA : collidedObjects)
        {
            for (const auto& objB : objA.second)
            {
                objA.first->notifyCollision(*objB);
                objB->notifyCollision(*objA.first);
            }
        }

        if (debugDrawer_)
        {
            world_->debugDrawWorld();
        }
    }
}