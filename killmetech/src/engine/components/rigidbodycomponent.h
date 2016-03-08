#ifndef _KILLME_RIGIDBODYCOMPONENT_H_
#define _KILLME_RIGIDBODYCOMPONENT_H_

#include "transformcomponent.h"
#include "../../physics/rigidbody.h"
#include <memory>

namespace killme
{
    class CollisionShape;

    /** The rigid body component defines of physics body into an actor */
    class RigidBodyComponent : public TransformComponent
    {
        KILLME_COMPONENT_DEFINE(RigidBodyComponent)

    private:
        struct Listener : public PhysicsListener
        {
            RigidBodyComponent* owner;
            void onMoved(const Vector3& pos, const Quaternion& q);
        };

        std::shared_ptr<RigidBody> body_;
        std::shared_ptr<Listener> listener_;

    public:
        /** Construct */
        RigidBodyComponent(const std::shared_ptr<CollisionShape>& shape, float mass);

        void onTranslated();
        void onRotated();

        void onActivate();
        void onDeactivate();
    };
}

#endif