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

    /** Rigid body */
    class RigidBody
    {
    private:
        std::unique_ptr<btRigidBody> body_;
        std::unique_ptr<btMotionState> motionState_;
        std::shared_ptr<CollisionShape> shape_;

    public:
        /** Construct with a body and mass */
        RigidBody(const std::shared_ptr<CollisionShape> shape, float mass);

        /** Transform modifiers */
        Vector3 getPosition() const;
        void setPosition(const Vector3& pos);

        Quaternion getOrientation() const;
        void setOrientation(const Quaternion& q);

        /** Return the bullet body */
        btRigidBody* getBtBody();
    };
}

#endif