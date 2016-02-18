#include "rigidbodycomponent.h"
#include "../physics.h"
#include "../processes.h"
#include "../../physics/rigidbody.h"

namespace killme
{
    RigidBodyComponent::RigidBodyComponent(const std::shared_ptr<CollisionShape>& shape, float mass)
        : body_(std::make_shared<RigidBody>(shape, mass))
        , process_()
    {
    }

    void RigidBodyComponent::onAttached()
    {
        TransformComponent::onAttached();
        Physics::addRigidBody(body_);
        body_->setPosition(getWorldPosition());
        body_->setOrientation(getWorldOrientation());
        process_ = Processes::start([&] { reflectPhysics(); }, PROCESS_PRIORITY_PHYSICS);
    }

    void RigidBodyComponent::onDettached()
    {
        TransformComponent::onDettached();
        Physics::removeRigidBody(body_);
        process_.kill();
    }

    void RigidBodyComponent::setPosition(const Vector3& pos)
    {
        TransformComponent::setPosition(pos);
        body_->setPosition(getWorldPosition());
    }

    void RigidBodyComponent::setOrientation(const Quaternion& q)
    {
        TransformComponent::setOrientation(q);
        body_->setOrientation(getWorldOrientation());
    }

    void RigidBodyComponent::reflectPhysics()
    {
        setWorldPosition(body_->getPosition());
        setWorldOrientation(body_->getOrientation());
    }
}