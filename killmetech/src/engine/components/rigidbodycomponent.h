#ifndef _KILLME_RIGIDBODYCOMPONENT_H_
#define _KILLME_RIGIDBODYCOMPONENT_H_

#include "transformcomponent.h"
#include "../../processes/process.h"
#include <memory>

namespace killme
{
    class RigidBody;
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
        /** Construct */
        RigidBodyComponent(const std::shared_ptr<CollisionShape>& shape, float mass);

        void onAttached();
        void onDettached();

        void setPosition(const Vector3& pos);
        void setOrientation(const Quaternion& q);

    private:
        void reflectPhysics();
    };
}

#endif