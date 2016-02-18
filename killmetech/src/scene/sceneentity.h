#ifndef _KILLME_SCENEENTITY_H_
#define _KILLME_SCENEENTITY_H_

#include <memory>

namespace killme
{
    class SceneNode;
    class RenderQueue;

    /** The scene entity */
    class SceneEntity
    {
    private:
        std::weak_ptr<SceneNode> owner_;

    public:
        /** Fro drived classes */
        virtual ~SceneEntity() = default;

        /** Sets the owner */
        void setOwner(const std::weak_ptr<SceneNode>& owner) { owner_ = owner; }

        /** Returns the owner */
        std::weak_ptr<SceneNode> getOwner() { return owner_; }

        /** Accept render queue */
        virtual void collectRenderer(RenderQueue&) {}
    };
}

#endif