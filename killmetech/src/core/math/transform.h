#ifndef _KILLME_TRANSFORM_H_
#define _KILLME_TRANSFORM_H_

#include "vector3.h"
#include "quaternion.h"
#include "../utility.h"
#include <unordered_set>
#include <memory>
#include <stack>

namespace killme
{
    class Matrix44;

    /** Transform node */
    class Transform
    {
    private:
        Vector3 position_;
        Quaternion orientation_;
        Vector3 scale_;
        std::weak_ptr<Transform> parent_;
        std::unordered_set<std::shared_ptr<Transform>> children_;

        mutable bool needUpdate_;
        mutable Vector3 worldPosition_;
        mutable Quaternion worldOrientation_;
        mutable Vector3 worldScale_;

    public:
        /** Construct */
        Transform();

        /** For drived classes */
        virtual ~Transform() = default;

        /** Return the parent */
        std::shared_ptr<const Transform> getParent() const;

        /** ditto */
        std::shared_ptr<Transform> getParent();

        // For addChildNode()
        void setParent(const std::weak_ptr<Transform>& parent);

        // ditto
        void addChild(const std::shared_ptr<Transform>& child);

        // For removeChildNode()
        void removeChild(const std::shared_ptr<Transform>& child);

        /** Return count of child */
        size_t getNumChildren() const;

        /** Return the children */
        /// NOTE: Returned range is destroyed if modify children after get range 
        auto getChildren()
            -> decltype(constRange(children_))
        {
            return constRange(children_);
        }

        /** Return the local relative position */
        Vector3 getPosition() const;

        /** Return the world relative position */
        Vector3 getWorldPosition() const;

        /** Set local relative position */
        virtual void setPosition(const Vector3& pos);

        /** Set world relative position */
        void setWorldPosition(const Vector3& wpos);

        /** Return the local relative orientation */
        Quaternion getOrientation() const;

        /** Return the world relative orientation */
        Quaternion getWorldOrientation() const;

        /** Set local relative orientation */
        virtual void setOrientation(const Quaternion& q);

        /** Set world relative orientation */
        void setWorldOrientation(const Quaternion& wq);

        /** Return the local relative scale */
        Vector3 getScale() const;

        /** Return the world relative scale */
        Vector3 getWorldScale() const;

        /** Set local relative scale */
        void setScale(const Vector3& k);

        /** Set world relative scale */
        void setWorldScale(const Vector3& wk);

        /** Return the local matrix */
        Matrix44 getMatrix() const;

        /** Return the world matrix */
        Matrix44 getWorldMatrix() const;

    protected:
        /** Set update need */
        void setUpdateNeed(bool need);

    private:
        void update() const;
    };

    /** Add a child node */
    template <class T>
    std::shared_ptr<T> addChildNode(const std::shared_ptr<Transform>& parent, const std::shared_ptr<T>& child)
    {
        parent->addChild(child);
        child->setParent(parent);
        return child;
    }

    /** Remove a child node */
    void removeChildNode(const std::shared_ptr<Transform>& parent, const std::shared_ptr<Transform>& child);

    /** Convert a world position to local */
    Vector3 worldPositionToLocal(const Transform& base, const Vector3& wpos);

    /** Convert a local position to world */
    Vector3 localPositionToWorld(const Transform& base, const Vector3& lpos);

    /** Convert a world orientation to local */
    Quaternion worldOrientationToLocal(const Transform& base, const Quaternion& wq);

    /** Convert a local orientation to world */
    Quaternion localOrientationToWorld(const Transform& base, const Quaternion& lq);

    /** Convert a world scale to local */
    Vector3 worldScaleToLocal(const Transform& base, const Vector3& wk);

    /** Convert a local scale to world */
    Vector3 localScaleToWorld(const Transform& base, const Vector3& lk);

    /** Depth traverse */
    template <class Trans, class Fun>
    void depthTraverse(Trans& node, Fun fun)
    {
        std::stack<Trans*> stack;
        stack.emplace(&node);

        while (!stack.empty())
        {
            const auto top = stack.top();
            stack.pop();
            if (fun(*top))
            {
                for (const auto& child : top->getChildren())
                {
                    stack.emplace(static_cast<Trans*>(child.get()));
                }
            }
        }
    }

    /** ditto */
    template <class Trans, class Fun>
    void depthTraverse(const std::shared_ptr<Trans>& node, Fun fun)
    {
        std::stack<std::shared_ptr<Trans>> stack;
        stack.emplace(node);

        while (!stack.empty())
        {
            const auto top = stack.top();
            stack.pop();
            if (fun(top))
            {
                for (const auto& child : top->getChildren())
                {
                    stack.emplace(std::static_pointer_cast<Trans>(child));
                }
            }
        }
    }
}

#endif
