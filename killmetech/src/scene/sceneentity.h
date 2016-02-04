#ifndef _KILLME_SCENEENTITY_H_
#define _KILLME_SCENEENTITY_H_

#include <memory>

namespace killme
{
    class SceneNode;
    class SceneVisitor;

    /** Scene entity */
    class SceneEntity
    {
    private:
        std::weak_ptr<SceneNode> owner_;

    public:
        virtual ~SceneEntity() = default;

        /** Set owner */
        void setOwner(const std::shared_ptr<SceneNode>& owner) { owner_ = owner; }

        /** Returns owner node */
        std::shared_ptr<SceneNode> lockOwner() { return owner_.lock(); }

        /** Accept visitor */
        virtual bool accept(SceneVisitor& v) = 0;
    };
}

#endif