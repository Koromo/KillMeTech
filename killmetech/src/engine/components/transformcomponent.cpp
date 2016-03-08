#include "transformcomponent.h"

namespace killme
{
    TransformComponent::TransformComponent()
        : enableReceiveMove_(false)
        , preWorldPosition_()
    {
        enableBeginFrame();
    }

    void TransformComponent::enableReceiveMove(bool enable)
    {
        enableReceiveMove_ = enable;
    }

    Vector3 TransformComponent::getPreFrameWorldPosition() const
    {
        return preWorldPosition_;
    }

    void TransformComponent::setPosition(const Vector3& pos)
    {
        Transform::setPosition(pos);
        const auto& receivers = notifyMove();
        for (const auto& receiver : receivers)
        {
            receiver->onTranslated();
        }
    }

    void TransformComponent::setOrientation(const Quaternion& q)
    {
        Transform::setOrientation(q);
        const auto& receivers = notifyMove();
        for (const auto& receiver : receivers)
        {
            receiver->onRotated();
        }
    }

    void TransformComponent::setScale(const Vector3& k)
    {
        Transform::setScale(k);
        const auto& receivers = notifyMove();
        for (const auto& receiver : receivers)
        {
            receiver->onScaled();
        }
    }

    void TransformComponent::onBeginFrame()
    {
        preWorldPosition_ = getPosition();
    }

    Vector3 getWorldVelocity(const TransformComponent& transform, float dt_s)
    {
        const auto v = transform.getWorldPosition() - transform.getPreFrameWorldPosition();
        return v / dt_s;
    }
}