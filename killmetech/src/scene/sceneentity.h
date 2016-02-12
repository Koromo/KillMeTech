#ifndef _KILLME_SCENEENTITY_H_
#define _KILLME_SCENEENTITY_H_

#include <memory>

namespace killme
{
    class SceneNode;
    class SceneVisitor;

    /** The scene entity */
    class SceneEntity
    {
    private:
        std::weak_ptr<SceneNode> owner_;

    public:
        /** Fro drived classes */
        virtual ~SceneEntity() = default;

        /** Sets an owner */
        void setOwner(const std::weak_ptr<SceneNode>& owner) { owner_ = owner; }

        /** Returns the owner */
        std::shared_ptr<SceneNode> lockOwner() { return owner_.lock(); }

        /** Accepts a visitor */
        virtual bool accept(SceneVisitor& v) = 0;
    };
}

#endif