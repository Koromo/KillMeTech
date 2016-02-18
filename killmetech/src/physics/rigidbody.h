#ifndef _KILLME_RIGIDBODY_H_
#define _KILLME_RIGIDBODY_H_

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btMotionState.h>
#include <memory>

namespace killme
{
    class Vector3;
    class Quaternion;
    class CollisionShape;

    /** The rigid body */
    class RigidBody
    {
    private:
        std::unique_ptr<btRigidBody> body_;
        std::unique_ptr<btMotionState> motionState_;
        std::shared_ptr<CollisionShape> shape_;

    public:
        /** Constructs with a body and mass */
        RigidBody(const std::shared_ptr<CollisionShape> shape, float mass);

        /** Transform modifiers */
        Vector3 getPosition() const;
        void setPosition(const Vector3& pos);

        /** ditto */
        Quaternion getOrientation() const;
        void setOrientation(const Quaternion& q);

        /** Returns the bullet body */
        btRigidBody* getBtBody();
    };
}

#endif