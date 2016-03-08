#include "transform.h"
#include "matrix44.h"
#include <stack>

namespace killme
{
    Transform::Transform()
        : position_()
        , orientation_()
        , scale_(1, 1, 1)
        , parent_()
        , children_()
    {
    }

    std::shared_ptr<const Transform> Transform::lockParent() const
    {
        return parent_.lock();
    }

    std::shared_ptr<Transform> Transform::lockParent()
    {
        return parent_.lock();
    }

    void Transform::setParent(const std::weak_ptr<Transform>& parent)
    {
        parent_ = parent;
    }

    void Transform::addChild(const std::shared_ptr<Transform>& child)
    {
        children_.emplace(child);
    }

    bool Transform::removeChild(const std::shared_ptr<Transform>& child)
    {
        return children_.erase(child) > 0;
    }

    size_t Transform::getNumChildren() const
    {
        return children_.size();
    }

    Vector3 Transform::getPosition() const
    {
        return position_;
    }

    Vector3 Transform::getWorldPosition() const
    {
        return detail::getWorldTransform(*this).position;
    }

    void Transform::setPosition(const Vector3& pos)
    {
        position_ = pos;
    }

    void Transform::setWorldPosition(const Vector3& wpos)
    {
        setPosition(fromWorldPosition(wpos));
    }

    Quaternion Transform::getOrientation() const
    {
        return orientation_;
    }

    Quaternion Transform::getWorldOrientation() const
    {
        return detail::getWorldTransform(*this).orientation;
    }

    void Transform::setOrientation(const Quaternion& q)
    {
        orientation_ = q;
    }

    void Transform::setWorldOrientation(const Quaternion& wq)
    {
        setOrientation(fromWorldOrientation(wq));
    }

    Vector3 Transform::getScale() const
    {
        return scale_;
    }

    Vector3 Transform::getWorldScale() const
    {
        return detail::getWorldTransform(*this).scale;
    }

    void Transform::setScale(const Vector3& k)
    {
        scale_ = k;
    }

    void Transform::setWorldScale(const Vector3& wk)
    {
        setScale(fromWorldScale(wk));
    }

    Vector3 Transform::fromWorldPosition(const Vector3& wpos) const
    {
        if (parent_.expired())
        {
            return wpos;
        }
        const auto world = detail::getWorldTransform(*parent_.lock().get());
        return inverse(world.orientation) * invScale(wpos - world.position, world.scale);
    }

    Vector3 Transform::toWorldPosition(const Vector3& pos) const
    {
        if (parent_.expired())
        {
            return pos;
        }
        const auto world = detail::getWorldTransform(*parent_.lock().get());
        return world.position + world.orientation * killme::scale(pos, world.scale);
    }

    Quaternion Transform::fromWorldOrientation(const Quaternion& wq) const
    {
        if (parent_.expired())
        {
            return wq;
        }
        const auto world = detail::getWorldTransform(*parent_.lock().get());
        return wq * inverse(world.orientation);
    }

    Quaternion Transform::toWorldOrientation(const Quaternion& q) const
    {
        if (parent_.expired())
        {
            return q;
        }
        const auto world = detail::getWorldTransform(*parent_.lock().get());
        return q * world.orientation;
    }

    Vector3 Transform::fromWorldScale(const Vector3& wk) const
    {
        if (parent_.expired())
        {
            return wk;
        }
        const auto world = detail::getWorldTransform(*parent_.lock().get());
        return invScale(wk, world.scale);
    }

    Vector3 Transform::toWorldScale(const Vector3& k) const
    {
        if (parent_.expired())
        {
            return k;
        }
        const auto world = detail::getWorldTransform(*parent_.lock().get());
        return killme::scale(k, world.scale);
    }

    Matrix44 Transform::getMatrix() const
    {
        return makeTransformMatrix(scale_, orientation_, position_);
    }

    Matrix44 Transform::getWorldMatrix() const
    {
        const auto world = detail::getWorldTransform(*this);
        return makeTransformMatrix(world.scale, world.orientation, world.position);
    }

    void removeChildNode(const std::shared_ptr<Transform>& parent, const std::shared_ptr<Transform>& child)
    {
        if (parent->removeChild(child))
        {
            child->setParent(std::shared_ptr<Transform>());
        }
    }

    detail::WorldTransform detail::getWorldTransform(const Transform& node)
    {
        std::stack<const Transform*> stack;
        stack.emplace(&node);

        auto parent = node.lockParent();
        while (parent)
        {
            stack.emplace(parent.get());
            parent = parent->lockParent();
        }

        WorldTransform world;

        const auto root = stack.top();
        world.position = root->getPosition();
        world.orientation = root->getOrientation();
        world.scale = root->getScale();
        stack.pop();

        while (!stack.empty())
        {
            const auto n = stack.top();
            world.position += (world.orientation * scale(n->getPosition(), world.scale));
            world.orientation = n->getOrientation() * world.orientation;
            world.scale = killme::scale(world.scale, n->getScale());
            stack.pop();
        }

        return world;
    }
}