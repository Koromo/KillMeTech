#ifndef _KILLME_SCENENODE_H_
#define _KILLME_SCENENODE_H_

#include "sceneentity.h"
#include "scenevisitor.h"
#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include <vector>
#include <memory>
#include <stack>
#include <queue>

namespace killme
{
    class Matrix44;
    
    namespace detail
    {
        inline std::shared_ptr<SceneNode> top(std::stack<std::shared_ptr<SceneNode>> c)
        {
            return c.top();
        }

        inline std::shared_ptr<SceneNode> top(std::queue<std::shared_ptr<SceneNode>> c)
        {
            return c.front();
        }
    }

    /** The transform node */
    class SceneNode : public std::enable_shared_from_this<SceneNode>
    {
    private:
        Vector3 position_;
        Quaternion orientation_;
        Vector3 scale_;
        std::weak_ptr<SceneNode> parent_;
        std::vector<std::shared_ptr<SceneNode>> children_;
        std::shared_ptr<SceneEntity> entity_;

    public:
        /** Constructs */
        SceneNode(const std::weak_ptr<SceneNode>& parent);

        /** Creates the child node */
        std::shared_ptr<SceneNode> createChild();

        /** Creates the entity */
        template <class T, class... Args>
        std::shared_ptr<T> attachEntity(Args... args)
        {
            const auto e = std::make_shared<T>(args...);
            e->setOwner(shared_from_this());
            entity_ = e;
            return e;
        }

        /** Updates the transform */
        void setPosition(const Vector3& pos);
        void setOrientation(const Quaternion& q);
        void setScale(const Vector3& k);

        /** Returns the world matrix */
        Matrix44 getWorldMatrix() const;

        /** Traverse scene */
        bool depthTraverse(SceneVisitor& v);
        bool widthTraverse(SceneVisitor& v);

    private:
        template <class C>
        bool traverseImpl(SceneVisitor& v, C& c)
        {
            c.push(shared_from_this());

            while (!c.empty())
            {
                const auto node = detail::top(c);
                c.pop();

                if (!node->entity_)
                {
                    if (v(node, nullptr))
                    {
                        return true;
                    }
                }
                else if (node->entity_->accept(v))
                {
                    return true;
                }

                for (const auto& child : node->children_)
                {
                    c.push(child);
                }
            }

            return false;
        }
    };
}

#endif
