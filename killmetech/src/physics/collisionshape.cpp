#include "collisionshape.h"
#include "bulletsupport.h"
#include <BulletCollision/CollisionShapes/btStaticPlaneShape.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <LinearMath/btVector3.h>

namespace killme
{
    CollisionShape::CollisionShape(btCollisionShape* shape, ShapeType type)
        : type_(type)
        , shape_(shape)
    {
    }

    ShapeType CollisionShape::getType() const
    {
        return type_;
    }

    btCollisionShape* CollisionShape::getBtShape()
    {
        return shape_.get();
    }

    std::shared_ptr<CollisionShape> createStaticPlaneShape(const Vector3& normal)
    {
        const btVector3 n = to<btVector3>(normal);
        return std::make_shared<CollisionShape>(new btStaticPlaneShape(n, 0), ShapeType::static_);
    }

    std::shared_ptr<CollisionShape> createBoxShape(float x, float y, float z)
    {
        const btVector3 v(x * 0.5f, y * 0.5f, z * 0.5f);
        return std::make_shared<CollisionShape>(new btBoxShape(v), ShapeType::dynamic);
    }
}