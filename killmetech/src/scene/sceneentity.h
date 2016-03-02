#ifndef _KILLME_SCENEENTITY_H_
#define _KILLME_SCENEENTITY_H_

#include <memory>

namespace killme
{
    class SceneNode;
    class RenderQueue;

    /** Scene entity */
    class SceneEntity
    {
    private:
        std::weak_ptr<SceneNode> owner_;

    public:
        /** For drived classes */
        virtual ~SceneEntity() = default;

        /** Set the owner */
        void setOwner(const std::weak_ptr<SceneNode>& owner) { owner_ = owner; }

        /** Return the owner */
        std::weak_ptr<SceneNode> getOwner() { return owner_; }

        /** Accept render queue */
        virtual void collectRenderer(RenderQueue&) {}
    };
}

#endif