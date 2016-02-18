#ifndef _KILLME_RIGIDBODYCOMPONENT_H_
#define _KILLME_RIGIDBODYCOMPONENT_H_

#include "transformcomponent.h"
#include "physics.h"
#include "processes.h"
#include "../processes/process.h"
#include "../physics/rigidbody.h"
#include <memory>

namespace killme
{
    class CollisionShape;
    class Vector3;
    class Quaternion;

    /** The rigid body component adds function of physics simulation into an actor */
    class RigidBodyComponent : public TransformComponent
    {
    private:
        std::shared_ptr<RigidBody> body_;
        Process process_;

    public:
        /** Constructs */
        RigidBodyComponent(const std::shared_ptr<CollisionShape>& shape, float mass)
            : body_(std::make_shared<RigidBody>(shape, mass))
            , process_()
        {
        }

        void onAttached()
        {
            TransformComponent::onAttached();
            Physics::addRigidBody(body_);
            body_->setPosition(getWorldPosition());
            body_->setOrientation(getWorldOrientation());
            process_ = Processes::start([&] { reflectPhysics(); }, PROCESS_PRIORITY_PHYSICS);
        }

        void onDettached()
        {
            TransformComponent::onDettached();
            Physics::removeRigidBody(body_);
            process_.kill();
        }

        void setPosition(const Vector3& pos)
        {
            TransformComponent::setPosition(pos);
            const auto p = getWorldPosition();
            body_->setPosition(getWorldPosition());
        }

        void setOrientation(const Quaternion& q)
        {
            TransformComponent::setOrientation(q);
            body_->setOrientation(getWorldOrientation());
        }

    private:
        void reflectPhysics()
        {
            setWorldPosition(body_->getPosition());
            setWorldOrientation(body_->getOrientation());
        }
    };
}

#endif