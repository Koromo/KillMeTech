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
        if (shape->getType() == ShapeType::static_)
        {
            mass = 0;
        }

        const auto btShape = shape_->getBtShape();
        btVector3 inertia(0, 0, 0);
        if (mass > 0)
        {
            btShape->calculateLocalInertia(mass, inertia);
        }
        btRigidBody::btRigidBodyConstructionInfo ci(mass, motionState_.get(), btShape, inertia);
        body_ = std::make_unique<btRigidBody>(ci);
    }

    Vector3 RigidBody::getPosition() const
    {
        auto trans = body_->getCenterOfMassTransform();
        return to<Vector3>(trans.getOrigin());
    }

    void RigidBody::setPosition(const Vector3& pos)
    {
        auto trans = body_->getCenterOfMassTransform();
        trans.setOrigin(to<btVector3>(pos));
        body_->setCenterOfMassTransform(trans);
        body_->activate(true);
    }

    Quaternion RigidBody::getOrientation() const
    {
        auto trans = body_->getCenterOfMassTransform();
        return to<Quaternion>(trans.getRotation());
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