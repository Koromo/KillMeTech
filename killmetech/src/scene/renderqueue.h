#ifndef _KILLME_RENDERQUEUE_H_
#define _KILLME_RENDERQUEUE_H_

#include "material.h"
#include "../core/math/matrix44.h"
#include <queue>
#include <memory>

namespace killme
{
    class VertexData;
    class Material;

    /** Render element in queue */
    struct RenderElement
    {
        std::shared_ptr<VertexData> vertices;
        std::shared_ptr<Material> material;
        Matrix44 worldMatrix; // Transposed
    };

    /** Render queue */
    class RenderQueue
    {
    private:
        struct Less
        {
            bool operator ()(const std::shared_ptr<const RenderElement>& a,
                const std::shared_ptr<const RenderElement>& b)
            {
                return a->material->getPriority() < b->material->getPriority();
            }
        };
        std::priority_queue<std::shared_ptr<const RenderElement>,
            std::vector<std::shared_ptr<const RenderElement>>, Less> queue_;

    public:
        /** Push a render element */
        void push(const std::shared_ptr<const RenderElement>& e) { queue_.emplace(e); }

        /** Pop a render element */
        std::shared_ptr<const RenderElement> pop() { const auto e = queue_.top(); queue_.pop(); return e; }

        /** Whether queue is empty or not */
        bool empty() const { return queue_.empty(); }
    };
}

#endif