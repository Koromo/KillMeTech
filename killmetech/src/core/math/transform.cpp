#include "transform.h"
#include "matrix44.h"
#include <cassert>

namespace killme
{
    Transform::Transform()
        : position_()
        , orientation_()
        , scale_(1, 1, 1)
        , parent_()
        , children_()
        , needUpdate_(false)
        , worldPosition_()
        , worldOrientation_()
        , worldScale_(1, 1, 1)
    {
    }

    std::shared_ptr<const Transform> Transform::getParent() const
    {
        return parent_.lock();
    }

    std::shared_ptr<Transform> Transform::getParent()
    {
        return parent_.lock();
    }

    void Transform::setParent(const std::weak_ptr<Transform>& parent)
    {
        parent_ = parent;
        needUpdate_ = true;
    }

    void Transform::addChild(const std::shared_ptr<Transform>& child)
    {
        assert((!child->getParent() || child->getParent().get() == this) && "Child transform node is already linked to an other node.");
        children_.emplace(child);
    }
    
    void Transform::removeChild(const std::shared_ptr<Transform>& child)
    {
        if (children_.erase(child) > 0)
        {
            child->parent_.reset();
        }
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
        update();
        return worldPosition_;
    }

    void Transform::setPosition(const Vector3& pos)
    {
        position_ = pos;
        needUpdate_ = true;
    }

    void Transform::setWorldPosition(const Vector3& wpos)
    {
        const auto parent = getParent();
        if (!parent)
        {
            setPosition(wpos);
        }
        else
        {
            setPosition(worldPositionToLocal(*parent, wpos));
        }
    }

    Quaternion Transform::getOrientation() const
    {
        return orientation_;
    }

    Quaternion Transform::getWorldOrientation() const
    {
        update();
        return worldOrientation_;
    }

    void Transform::setOrientation(const Quaternion& q)
    {
        orientation_ = q;
        needUpdate_ = true;
    }

    void Transform::setWorldOrientation(const Quaternion& wq)
    {
        const auto parent = getParent();
        if (!parent)
        {
            setOrientation(wq);
        }
        else
        {
            setOrientation(worldOrientationToLocal(*parent, wq));
        }
    }

    Vector3 Transform::getScale() const
    {
        return scale_;
    }

    Vector3 Transform::getWorldScale() const
    {
        update();
        return worldScale_;
    }

    void Transform::setScale(const Vector3& k)
    {
        scale_ = k;
        needUpdate_ = true;
    }

    void Transform::setWorldScale(const Vector3& wk)
    {
        const auto parent = getParent();
        if (!parent)
        {
            setScale(wk);
        }
        else
        {
            setScale(worldScaleToLocal(*parent, wk));
        }
    }

    Matrix44 Transform::getMatrix() const
    {
        return makeTransformMatrix(scale_, orientation_, position_);
    }

    Matrix44 Transform::getWorldMatrix() const
    {
        update();
        return makeTransformMatrix(worldScale_, worldOrientation_, worldPosition_);
    }

    void Transform::setUpdateNeed(bool need)
    {
        needUpdate_ = need;
    }

    void Transform::update() const
    {
        std::stack<const Transform*> pass;
        pass.emplace(this);
        
        auto parent = getParent();
        while (parent)
        {
            pass.emplace(parent.get());
            parent = parent->getParent();
        }
        while (!pass.empty() && !pass.top()->needUpdate_)
        {
            pass.pop();
        }

        if (pass.empty())
        {
            return;
        }

        if (!pass.top()->getParent())
        {
            pass.top()->worldPosition_ = pass.top()->position_;
            pass.top()->worldOrientation_ = pass.top()->orientation_;
            pass.top()->worldScale_ = pass.top()->scale_;
            pass.top()->needUpdate_ = false;
            pass.pop();
        }

        while (!pass.empty())
        {
            const auto node = pass.top();
            parent = node->getParent();
            node->worldPosition_ = parent->worldPosition_ +
                parent->worldOrientation_ * killme::scale(node->position_, parent->worldScale_);
            node->worldOrientation_ = node->orientation_ * parent->worldOrientation_;
            node->worldScale_ = killme::scale(parent->worldScale_, node->scale_);
            node->needUpdate_ = false;
            pass.pop();
        }
    }

    void removeChildNode(const std::shared_ptr<Transform>& parent, const std::shared_ptr<Transform>& child)
    {
        parent->removeChild(child);
    }

    Vector3 worldPositionToLocal(const Transform& base, const Vector3& wpos)
    {
        return inverse(base.getWorldOrientation()) * invScale(wpos - base.getWorldPosition(), base.getWorldScale());
    }

    Vector3 localPositionToWorld(const Transform& base, const Vector3& lpos)
    {
        return base.getWorldPosition() + base.getWorldOrientation() * scale(lpos, base.getWorldScale());
    }

    Quaternion worldOrientationToLocal(const Transform& base, const Quaternion& wq)
    {
        return wq * inverse(base.getWorldOrientation());
    }

    Quaternion localOrientationToWorld(const Transform& base, const Quaternion& lq)
    {
        return lq * base.getWorldOrientation();
    }

    Vector3 worldScaleToLocal(const Transform& base, const Vector3& wk)
    {
        return invScale(wk, base.getWorldScale());
    }

    Vector3 localScaleToWorld(const Transform& base, const Vector3& lk)
    {
        return scale(lk, base.getWorldScale());
    }
}