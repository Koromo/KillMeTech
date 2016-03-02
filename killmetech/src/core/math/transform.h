#ifndef _KILLME_TRANSFORM_H_
#define _KILLME_TRANSFORM_H_

#include "vector3.h"
#include "quaternion.h"
#include "matrix44.h"
#include "../utility.h"
#include <unordered_set>
#include <memory>
#include <stack>
#include <cassert>

namespace killme
{
    /** Transform */
    template <class T>
    class Transform : public std::enable_shared_from_this<T>
    {
    private:
        Vector3 position_;
        Quaternion orientation_;
        Vector3 scale_;
        std::weak_ptr<T> parent_;
        std::unordered_set<std::shared_ptr<T>> children_;

    public:
        /** Construct */
        Transform()
            : position_()
            , orientation_()
            , scale_(1, 1, 1)
            , parent_()
            , children_()
        {
        }

        /** For drived classes */
        virtual ~Transform() = default;

        /** Return the parent */
        std::shared_ptr<const T> lockParent() const
        {
            return parent_.lock();
        }

        std::shared_ptr<T> lockParent()
        {
            return parent_.lock();
        }

        /** Add the child node */
        virtual void addChild(const std::shared_ptr<T>& child)
        {
            assert(child->parent_.expired() && "Child transform is already linked to an other node.");
            child->parent_ = shared_from_this();
            children_.emplace(child);
        }

        /** Remove the child node */
        void removeChild(const std::shared_ptr<T>& child)
        {
            const auto n = children_.erase(child);
            if (n > 0)
            {
                child->parent_ = std::shared_ptr<T>();
            }
        }

        /** Return count of child */
        size_t getNumChildren() const
        {
            return children_.size();
        }

        /** Return the children */
        /// NOTE: Returned range is destroyed if modify children after get range 
        auto getChildren()
            -> decltype(makeRange(children_))
        {
            return makeRange(children_);
        }

        /** Return the local relative position */
        Vector3 getPosition() const
        {
            return position_;
        }

        /** Return the world relative position */
        Vector3 getWorldPosition() const
        {
            return getWorldTransform().position;
        }

        /** Set local relative position */
        virtual void setPosition(const Vector3& pos)
        {
            position_ = pos;
        }

        /** Set world relative position */
        void setWorldPosition(const Vector3& wpos)
        {
            setPosition(fromWorldPosition(wpos));
        }

        /** Return the local relative orientation */
        Quaternion getOrientation() const
        {
            return orientation_;
        }

        /** Return the world relative orientation */
        Quaternion getWorldOrientation() const
        {
            return getWorldTransform().orientation;
        }

        /** Set local relative orientation */
        virtual void setOrientation(const Quaternion& q)
        {
            orientation_ = q;
        }

        /** Set world relative orientation */
        void setWorldOrientation(const Quaternion& wq)
        {
            setOrientation(fromWorldOrientation(wq));
        }

        /** Return the local relative scale */
        Vector3 getScale() const
        {
            return scale_;
        }

        /** Return the world relative scale */
        Vector3 getWorldScale() const
        {
            return getWorldTransform().scale;
        }

        /** Set local relative scale */
        void setScale(const Vector3& k)
        {
            scale_ = k;
        }

        /** Set world relative scale */
        void setWorldScale(const Vector3& wk)
        {
            setScale(fromWorldScale(wk));
        }

        /** Convert the world position to local */
        Vector3 fromWorldPosition(const Vector3& wpos)
        {
            if (parent_.expired())
            {
                return wpos;
            }

            const auto parent = parent_.lock();
            const auto world = parent->getWorldTransform();
            return inverse(world.orientation) * invScale(wpos - world.position, world.scale);
        }

        /** Convert the local position to world */
        Vector3 toWorldPosition(const Vector3& pos)
        {
            if (parent_.expired())
            {
                return pos;
            }

            const auto parent = parent_.lock();
            const auto world = parent->getWorldTransform();
            return world.position + world.orientation * killme::scale(pos, world.scale);
        }

        /** Convert the world orientation to local */
        Quaternion fromWorldOrientation(const Quaternion& wq)
        {
            if (parent_.expired())
            {
                return wq;
            }

            const auto parent = parent_.lock();
            const auto world = parent->getWorldTransform();
            return wq * inverse(world.orientation);
        }

        /** Convert the local orientation to world */
        Quaternion toWorldOrientation(const Quaternion& q)
        {
            if (parent_.expired())
            {
                return q;
            }

            const auto parent = parent_.lock();
            const auto world = parent->getWorldTransform();
            return q * world.orientation;
        }

        /** Convert the world scale to local */
        Vector3 fromWorldScale(const Vector3& wk)
        {
            if (parent_.expired())
            {
                return wk;
            }

            const auto parent = parent_.lock();
            const auto world = parent->getWorldTransform();
            return invScale(wk, world.scale);
        }

        /** Convert the local scale to world */
        Vector3 toWorldScale(const Vector3& k)
        {
            if (parent_.expired())
            {
                return k;
            }

            const auto parent = parent_.lock();
            const auto world = parent->getWorldTransform();
            return killme::scale(k, world.scale);
        }

        /** Return the local matrix */
        Matrix44 getMatrix() const
        {
            return makeTransformMatrix(scale_, orientation_, position_);
        }

        /** Return the world matrix */
        Matrix44 getWorldMatrix() const
        {
            const auto world = getWorldTransform();
            return makeTransformMatrix(world.scale, world.orientation, world.position);
        }

    private:
        struct WorldTransform
        {
            Vector3 position;
            Quaternion orientation;
            Vector3 scale;
        };

        WorldTransform getWorldTransform() const
        {
            std::stack<std::shared_ptr<const Transform>> stack;
            stack.emplace(shared_from_this());

            auto parent = lockParent();
            while (parent)
            {
                stack.emplace(parent);
                parent = parent->lockParent();
            }

            WorldTransform world;

            const auto root = stack.top();
            world.position = root->position_;
            world.orientation = root->orientation_;
            world.scale = root->scale_;
            stack.pop();

            while (!stack.empty())
            {
                const auto trans = stack.top();
                world.position += (world.orientation * killme::scale(trans->position_, world.scale));
                world.orientation = trans->orientation_ * world.orientation;
                world.scale = killme::scale(world.scale, trans->scale_);
                stack.pop();
            }

            return world;
        }
    };
}

#endif
