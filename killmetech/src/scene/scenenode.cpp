#include "scenenode.h"
#include "../core/math/matrix44.h"
#include <stack>

namespace killme
{
    SceneNode::~SceneNode()
    {
        if (entity_)
        {
            entity_->setOwner(std::shared_ptr<SceneNode>());
        }
    }

    bool SceneNode::collectRenderer(RenderQueue& queue)
    {
        std::stack<std::shared_ptr<SceneNode>> stack;

        stack.emplace(shared_from_this());

        while (!stack.empty())
        {
            const auto node = stack.top();
            stack.pop();

            if (node->entity_)
            {
                node->entity_->collectRenderer(queue);
            }

            for (const auto& child : node->getChildren())
            {
                stack.emplace(child);
            }
        }

        return false;
    }
}