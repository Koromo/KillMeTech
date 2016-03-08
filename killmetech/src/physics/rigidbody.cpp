#include "rigidbody.h"
#include "collisionshape.h"
#include "bulletsupport.h"

namespace killme
{
    void RigidBody::MotionState::getWorldTransform(btTransform& worldTrans) const
    {
        worldTrans.setIdentity();
    }

    void RigidBody::MotionState::setWorldTransform(const btTransform& worldTrans)
    {
        if (owner->listener_)
        {
            const auto pos = to<Vector3>(worldTrans.getOrigin());
            const auto ori = to<Quaternion>(worldTrans.getRotation());
            owner->listener_->onMoved(pos, ori);
        }
    }

    RigidBody::RigidBody(const std::shared_ptr<CollisionShape> shape, float mass)
        : body_()
        , motionState_()
        , shape_(shape)
        , listener_()
    {
        motionState_.owner = this;

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
        btRigidBody::btRigidBodyConstructionInfo ci(mass, &motionState_, btShape, inertia);
        body_ = std::make_unique<btRigidBody>(ci);
    }

    void RigidBody::setListener(const std::shared_ptr<PhysicsListener>& listener)
    {
        listener_ = listener;
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