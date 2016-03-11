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
    class RigidBody;

    /** Physics listener */
    class PhysicsListener
    {
    public:
        virtual ~PhysicsListener() = default;
        virtual void onMoved(const Vector3& pos, const Quaternion& q) {}
        virtual void onCollided(RigidBody& collider) {}
    };

    /** Rigid body */
    class RigidBody
    {
    private:
        struct MotionState : public btMotionState
        {
            RigidBody* owner;
            void getWorldTransform(btTransform& worldTrans) const;
            void setWorldTransform(const btTransform& worldTrans);
        };

        std::unique_ptr<btRigidBody> body_;
        MotionState motionState_;
        std::shared_ptr<CollisionShape> shape_;
        std::shared_ptr<PhysicsListener> listener_;
        void* userPointer_;

    public:
        /** Construct with a body and mass */
        RigidBody(const std::shared_ptr<CollisionShape> shape, float mass);

        /** Set a listener */
        void setListener(const std::shared_ptr<PhysicsListener>& listener);

        /** Transform modifiers */
        void setPosition(const Vector3& pos);
        void setOrientation(const Quaternion& q);

        /** Notify cllision */
        void notifyCollision(RigidBody& collider);

        /** User pointer modifiers */
        void setUserPointer(void* p);
        void* getUserPointer();

        /** Return the bullet body */
        btRigidBody* getBtBody();
    };
}

#endif