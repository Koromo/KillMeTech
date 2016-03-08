#include "rigidbodycomponent.h"
#include "../level.h"
#include "../../physics/physicsworld.h"

namespace killme
{
    void RigidBodyComponent::Listener::onMoved(const Vector3& pos, const Quaternion& q)
    {
        owner->enableReceiveMove(false);
        owner->setWorldPosition(pos);
        owner->setWorldOrientation(q);
        owner->enableReceiveMove(true);
    }

    RigidBodyComponent::RigidBodyComponent(const std::shared_ptr<CollisionShape>& shape, float mass)
        : body_(std::make_shared<RigidBody>(shape, mass))
        , listener_(std::make_shared<Listener>())
    {
        listener_->owner = this;
        body_->setListener(listener_);
        enableReceiveMove(true);
    }

    void RigidBodyComponent::onTranslated()
    {
        body_->setPosition(getWorldPosition());
    }

    void RigidBodyComponent::onRotated()
    {
        body_->setOrientation(getWorldOrientation());
    }

    void RigidBodyComponent::onActivate()
    {
        getOwnerLevel().getPhysicsWorld().addRigidBody(body_);
    }

    void RigidBodyComponent::onDeactivate()
    {
        getOwnerLevel().getPhysicsWorld().removeRigidBody(body_);
    }
}