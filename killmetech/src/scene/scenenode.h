#ifndef _KILLME_SCENENODE_H_
#define _KILLME_SCENENODE_H_

#include "sceneentity.h"
#include "../core/math/transform.h"
#include <memory>
#include <utility>

namespace killme
{
    class SceneNode;
    class RenderQueue;

    /** Transform node */
    class SceneNode : public Transform<SceneNode>
    {
    private:
        std::shared_ptr<SceneEntity> entity_;

    public:
        /** Destruct */
        ~SceneNode();

        /** Create the entity */
        template <class T, class... Args>
        std::shared_ptr<T> attachEntity(Args&&... args)
        {
            const auto e = std::make_shared<T>(std::forward<Args>(args)...);
            e->setOwner(shared_from_this());
            entity_ = e;
            return e;
        }

        /** Collect renderer for render scene */
        bool collectRenderer(RenderQueue& queue);
    };
}

#endif
