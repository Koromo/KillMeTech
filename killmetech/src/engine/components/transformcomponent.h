#ifndef _KILLME_TRANSFORMCOMPONENT_H_
#define _KILLME_TRANSFORMCOMPONENT_H_

#include "actorcomponent.h"
#include "../actor.h"
#include "../../core/math/transform.h"
#include "../../core/math/vector3.h"

namespace killme
{
    class Process;

    /** The transform component defines transform into an actor. */
    class TransformComponent : public ActorComponent, public Transform
    {
        KILLME_COMPONENT_DEFINE_BEGIN(TransformComponent)
            KILLME_HOOK_LEVEL_EVENT(LEVEL_BeginFrame, &TransformComponent::onBeginFrame)
        KILLME_COMPONENT_DEFINE_END

    private:
        Vector3 preWorldPosition_;
        bool moveReceivable_;
        bool ignoreParentMove_;

    public:
        /** Construct */
        TransformComponent();

        /** If set to true, callback functions for move are enabled */
        void setMoveRecievable(bool enable);

        /** Whether recievable move or not */
        bool isMoveRecievable() const;

        /** If set to false, this transform ignore parent move */
        void setIgnoreParentMove(bool ignore);

        /** Whether this transform ignore parent move or not */
        bool isIgnoringParentMove() const;

        /** Return world relative position when frame begin */
        Vector3 getPreWorldPosition() const;

        void setPosition(const Vector3& pos);
        void setOrientation(const Quaternion& q);
        void setScale(const Vector3& k);

        /** Called on moved */
        virtual void onTranslated() {}
        virtual void onRotated() {}
        virtual void onScaled() {}

    private:
        void onBeginFrame();
    };

    /** Return world relative world from pre frame */
    Vector3 getWorldVelocity(const TransformComponent& transform, float dt_s);

    /** Add a child transformed component */
    template <class T>
    std::shared_ptr<T> attachChildTransform(const std::shared_ptr<TransformComponent>& parent, const std::shared_ptr<T>& child)
    {
        addChildNode(parent, child);
        if (parent->hasOwner())
        {
            parent->getOwnerActor().attachTransformedComponentImpl(child);
        }
        return child;
    }
}

#endif