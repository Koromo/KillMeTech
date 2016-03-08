#ifndef _KILLME_TRANSFORMCOMPONENT_H_
#define _KILLME_TRANSFORMCOMPONENT_H_

#include "actorcomponent.h"
#include "../../core/utility.h"
#include "../../core/math/transform.h"
#include "../../core/math/vector3.h"
#include <stack>
#include <memory>
#include <utility>

namespace killme
{
    class Process;

    /** The transform component defines transform into an actor. */
    class TransformComponent : public ActorComponent, public Transform
    {
        KILLME_COMPONENT_DEFINE(TransformComponent)

    private:
        bool enableReceiveMove_;
        Vector3 preWorldPosition_;

    public:
        /** Construct */
        TransformComponent();

        /** Enable receive move */
        void enableReceiveMove(bool enable);

        /** Return world relative pre frame position */
        Vector3 getPreFrameWorldPosition() const;

        /** Called on moved */
        virtual void onTranslated() {}
        virtual void onRotated() {}
        virtual void onScaled() {}

        void setPosition(const Vector3& pos);
        void setOrientation(const Quaternion& q);
        void setScale(const Vector3& k);

        void onBeginFrame();

    private:
        auto notifyMove()
            -> decltype(emplaceRange(std::vector<TransformComponent*>()))
        {
            std::vector<TransformComponent*> receivers;
            std::stack<TransformComponent*> stack;
            stack.emplace(this);

            while (!stack.empty())
            {
                const auto top = stack.top();
                stack.pop();

                if (top->enableReceiveMove_)
                {
                    receivers.emplace_back(top);
                }

                for (const auto& c : top->getChildren())
                {
                    stack.emplace(static_cast<TransformComponent*>(c.get()));
                }
            }

            return emplaceRange(std::move(receivers));
        }
    };

    /** Return world relative world from pre frame */
    Vector3 getWorldVelocity(const TransformComponent& transform, float dt_s);
}

#endif