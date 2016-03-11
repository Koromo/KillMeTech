#include "cameracomponent.h"
#include "../level.h"
#include "../graphicssystem.h"
#include "../../scene/camera.h"
#include "../../scene/scene.h"

namespace killme
{
    CameraComponent::CameraComponent()
        : camera_(std::make_shared<Camera>(graphicsSystem.getClientViewport()))
        , isMainCamera_(false)
    {
        setMoveRecievable(true);
    }

    void CameraComponent::setViewport(const Viewport& vp)
    {
        camera_->setViewport(vp);
    }

    void CameraComponent::setFovX(float rad)
    {
        camera_->setFovX(rad);
    }

    void CameraComponent::setAspectRate(float aspect)
    {
        camera_->setAspectRate(aspect);
    }

    void CameraComponent::setNearZ(float z)
    {
        camera_->setNearZ(z);
    }

    void CameraComponent::setFarZ(float z)
    {
        camera_->setFarZ(z);
    }

    void CameraComponent::setEnable(bool enable)
    {
        isMainCamera_ = enable;
        if (isActive())
        {
            if (enable)
            {
                getOwnerLevel().getGraphicsWorld().setMainCamera(camera_);
            }
            else if (getOwnerLevel().getGraphicsWorld().getMainCamera() == camera_)
            {
                getOwnerLevel().getGraphicsWorld().setMainCamera(nullptr);
            }
        }
    }

    void CameraComponent::onTranslated()
    {
        camera_->setPosition(getWorldPosition());
    }

    void CameraComponent::onRotated()
    {
        camera_->setOrientation(getWorldOrientation());
    }

    void CameraComponent::onActivate()
    {
        getOwnerLevel().getGraphicsWorld().addCamera(camera_);
        if (isMainCamera_)
        {
            getOwnerLevel().getGraphicsWorld().setMainCamera(camera_);
        }
    }

    void CameraComponent::onDeactivate()
    {
        setEnable(false);
        getOwnerLevel().getGraphicsWorld().removeCamera(camera_);
    }
}