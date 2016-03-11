#include "rigidbodycomponent.h"
#include "../actor.h"
#include "../level.h"
#include "../../physics/physicsworld.h"

namespace killme
{
    Collider::Collider(RigidBodyComponent* body)
        : body_(body)
    {
    }

    RigidBodyComponent& Collider::getBody() const
    {
        return *body_;
    }

    Actor& Collider::getActor() const
    {
        return body_->getOwnerActor();
    }

    void RigidBodyComponent::Listener::onMoved(const Vector3& pos, const Quaternion& q)
    {
        owner->setMoveRecievable(false);
        owner->setWorldPosition(pos);
        owner->setWorldOrientation(q);
        owner->setMoveRecievable(true);
    }

    void RigidBodyComponent::Listener::onCollided(RigidBody& collider)
    {
        owner->getOwnerActor().emit(ACTOR_Collided, Collider(owner), Collider(static_cast<RigidBodyComponent*>(collider.getUserPointer())));
    }

    RigidBodyComponent::RigidBodyComponent(const std::shared_ptr<CollisionShape>& shape, float mass)
        : body_(std::make_shared<RigidBody>(shape, mass))
        , listener_(std::make_shared<Listener>())
    {
        listener_->owner = this;
        body_->setListener(listener_);
        body_->setUserPointer(this);
        setMoveRecievable(true);
        setIgnoreParentMove(true);
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