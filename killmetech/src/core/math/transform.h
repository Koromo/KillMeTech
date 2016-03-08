#ifndef _KILLME_TRANSFORM_H_
#define _KILLME_TRANSFORM_H_

#include "vector3.h"
#include "quaternion.h"
#include "../utility.h"
#include <unordered_set>
#include <memory>
#include <cassert>

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

    public:
        /** Construct */
        Transform();

        /** For drived classes */
        virtual ~Transform() = default;

        /** Return the parent */
        std::shared_ptr<const Transform> lockParent() const;

        /** ditto */
        std::shared_ptr<Transform> lockParent();

        // For addChildNode()
        void setParent(const std::weak_ptr<Transform>& parent);

        // ditto
        void addChild(const std::shared_ptr<Transform>& child);

        // For removeChildNode()
        bool removeChild(const std::shared_ptr<Transform>& child);

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

        /** Convert the world position to local */
        Vector3 fromWorldPosition(const Vector3& wpos) const;

        /** Convert the local position to world */
        Vector3 toWorldPosition(const Vector3& pos) const;

        /** Convert the world orientation to local */
        Quaternion fromWorldOrientation(const Quaternion& wq) const;

        /** Convert the local orientation to world */
        Quaternion toWorldOrientation(const Quaternion& q) const;

        /** Convert the world scale to local */
        Vector3 fromWorldScale(const Vector3& wk) const;

        /** Convert the local scale to world */
        Vector3 toWorldScale(const Vector3& k) const;

        /** Return the local matrix */
        Matrix44 getMatrix() const;

        /** Return the world matrix */
        Matrix44 getWorldMatrix() const;
    };

    /** Add a child node */
    template <class T>
    std::shared_ptr<T> addChildNode(const std::shared_ptr<Transform>& parent, const std::shared_ptr<T>& child)
    {
        assert(!child->lockParent() && "Child transform node is already linked to an other node.");
        parent->addChild(child);
        child->setParent(parent);
        return child;
    }

    /** Remove a child node */
    void removeChildNode(const std::shared_ptr<Transform>& parent, const std::shared_ptr<Transform>& child);

    namespace detail
    {
        struct WorldTransform
        {
            Vector3 position;
            Quaternion orientation;
            Vector3 scale;
        };

        WorldTransform getWorldTransform(const Transform& node);
    }
}

#endif
