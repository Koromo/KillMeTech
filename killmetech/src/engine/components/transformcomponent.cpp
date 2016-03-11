#include "transformcomponent.h"
#include <vector>
#include <utility>

namespace killme
{
    TransformComponent::TransformComponent()
        : preWorldPosition_()
        , moveReceivable_(false)
        , ignoreParentMove_(false)
    {
    }

    void TransformComponent::setMoveRecievable(bool enable)
    {
        moveReceivable_ = enable;
    }

    bool TransformComponent::isMoveRecievable() const
    {
        return moveReceivable_;
    }

    bool TransformComponent::isIgnoringParentMove() const
    {
        return ignoreParentMove_;
    }

    void TransformComponent::setIgnoreParentMove(bool ignore)
    {
        ignoreParentMove_ = ignore;
    }

    Vector3 TransformComponent::getPreWorldPosition() const
    {
        return preWorldPosition_;
    }

    void TransformComponent::setPosition(const Vector3& pos)
    {
        std::vector<std::pair<TransformComponent*, Vector3>> ignore;
        std::vector<TransformComponent*> receive;

        depthTraverse(*this, [&](TransformComponent& n) {
            if (&n != this && n.isIgnoringParentMove())
            {
                ignore.emplace_back(&n, n.getWorldPosition());
                return false;
            }
            if (n.isMoveRecievable())
            {
                receive.emplace_back(&n);
            }
            return true;
        });

        Transform::setPosition(pos);

        for (const auto& i : ignore)
        {
            const auto lpos = worldPositionToLocal(*i.first->getParent(), i.second);
            i.first->Transform::setPosition(lpos);
            i.first->setUpdateNeed(false);
        }
        for (const auto r : receive)
        {
            r->onTranslated();
        }
    }

    void TransformComponent::setOrientation(const Quaternion& q)
    {
        std::vector<std::pair<TransformComponent*, Quaternion>> ignore;
        std::vector<TransformComponent*> receive;

        depthTraverse(*this, [&](TransformComponent& n) {
            if (&n != this && n.isIgnoringParentMove())
            {
                ignore.emplace_back(&n, n.getWorldOrientation());
                return false;
            }
            if (n.isMoveRecievable())
            {
                receive.emplace_back(&n);
            }
            return true;
        });

        Transform::setOrientation(q);

        for (const auto& i : ignore)
        {
            const auto lq = worldOrientationToLocal(*i.first->getParent(), i.second);
            i.first->Transform::setOrientation(lq);
            i.first->setUpdateNeed(false);
        }
        for (const auto r : receive)
        {
            r->onRotated();
        }
    }

    void TransformComponent::setScale(const Vector3& k)
    {
        std::vector<std::pair<TransformComponent*, Vector3>> ignore;
        std::vector<TransformComponent*> receive;

        depthTraverse(*this, [&](TransformComponent& n) {
            if (&n != this && n.isIgnoringParentMove())
            {
                ignore.emplace_back(&n, n.getWorldScale());
                return false;
            }
            if (n.isMoveRecievable())
            {
                receive.emplace_back(&n);
            }
            return true;
        });

        Transform::setScale(k);

        for (const auto& i : ignore)
        {
            const auto lk = worldScaleToLocal(*i.first->getParent(), i.second);
            i.first->Transform::setScale(lk);
            i.first->setUpdateNeed(false);
        }
        for (const auto r : receive)
        {
            r->onScaled();
        }
    }

    void TransformComponent::onBeginFrame()
    {
        preWorldPosition_ = getWorldPosition();
    }

    Vector3 getWorldVelocity(const TransformComponent& transform, float dt_s)
    {
        const auto v = transform.getWorldPosition() - transform.getPreWorldPosition();
        return v / dt_s;
    }
}