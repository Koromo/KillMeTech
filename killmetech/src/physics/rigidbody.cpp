#include "rigidbody.h"
#include "collisionshape.h"
#include "bulletsupport.h"
#include <LinearMath/btDefaultMotionState.h>

namespace killme
{
    RigidBody::RigidBody(const std::shared_ptr<CollisionShape> shape, float mass)
        : body_()
        , motionState_(std::make_unique<btDefaultMotionState>())
        , shape_(shape)
    {
        const auto btShape = shape_->getBtShape();
        btVector3 inertia(0, 0, 0);
        if (mass > 0)
        {
            btShape->calculateLocalInertia(mass, inertia);
        }
        btRigidBody::btRigidBodyConstructionInfo ci(mass, motionState_.get(), btShape, inertia);
        body_ = std::make_unique<btRigidBody>(ci);
    }

    void RigidBody::setPosition(const Vector3& pos)
    {
        auto trans = body_->getCenterOfMassTransform();
        trans.setOrigin(to<btVector3>(pos));
        body_->setCenterOfMassTransform(trans);
        body_->activate(true);
    }

    void RigidBody::setOrientation(const Quaternion& q)
    {
        auto trans = body_->getCenterOfMassTransform();
        trans.setRotation(to<btQuaternion>(q));
        body_->setCenterOfMassTransform(trans);
        body_->activate(true);
    }

    btRigidBody* RigidBody::getBtBody()
    {
        return body_.get();
    }
}