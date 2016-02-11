#ifndef _KILLME_COLLISIONSHAPE_H_
#define _KILLME_COLLISIONSHAPE_H_

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <memory>

namespace killme
{
    class Vector3;

    /** The shape type definition */
    enum class ShapeType
    {
        static_,
        dynamic,
    };

    /** The collision shape */
    class CollisionShape
    {
    private:
        ShapeType type_;
        std::unique_ptr<btCollisionShape> shape_;

    public:
        /** Constructs with a bullet shape */
        explicit CollisionShape(btCollisionShape* btShape, ShapeType type);

        /** Returns the shape type */
        ShapeType getType() const;

        /** Returns Bullet shape */
        btCollisionShape* getBtShape();
    };

    /** Create any shapes */
    std::shared_ptr<CollisionShape> createStaticPlaneShape(const Vector3& normal);
    std::shared_ptr<CollisionShape> createBoxShape(float x, float y, float z);
}

#endif