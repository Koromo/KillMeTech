#include "scenenode.h"
#include "../core/math/matrix44.h"

namespace killme
{
    SceneNode::SceneNode(const std::shared_ptr<SceneNode>& parent)
        : position_()
        , orientation_()
        , scale_(1, 1, 1)
        , parent_(parent)
        , children_()
        , entity_()
    {
    }

    std::shared_ptr<SceneNode> SceneNode::createChild()
    {
        const auto c = std::make_shared<SceneNode>(shared_from_this());
        children_.push_back(c);
        return c;
    }

    void SceneNode::setPosition(const Vector3& pos)
    {
        position_ = pos;
    }

    void SceneNode::setOrientation(const Quaternion& q)
    {
        orientation_ = q;
    }

    void SceneNode::setScale(const Vector3& k)
    {
        scale_ = k;
    }

    Matrix44 SceneNode::getWorldMatrix() const
    {
        std::stack<std::shared_ptr<const SceneNode>> stack;

        std::shared_ptr<const SceneNode> p = parent_.lock();
        while (p)
        {
            stack.push(p);
            p = p->parent_.lock();
        }
        stack.push(shared_from_this());

        Matrix44 m;
        while (!stack.empty())
        {
            const auto n = stack.top();
            m *= makeTransformMatrix(n->scale_, n->orientation_, n->position_);
            stack.pop();
        }

        return m;
    }

    bool SceneNode::depthTraverse(SceneVisitor& v)
    {
        return traverseImpl(v, std::stack<std::shared_ptr<SceneNode>>());
    }

    bool SceneNode::widthTraverse(SceneVisitor& v)
    {
        return traverseImpl(v, std::queue<std::shared_ptr<SceneNode>>());
    }
}