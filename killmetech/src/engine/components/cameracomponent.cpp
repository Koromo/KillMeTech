#include "cameracomponent.h"
#include "../level.h"
#include "../graphicssystem.h"
#include "../../scene/camera.h"
#include "../../scene/scene.h"

namespace killme
{
    CameraComponent::CameraComponent()
        : camera_(std::make_shared<Camera>(graphicsSystem.getClientViewport()))
        , setToMainCamera_(false)
    {
        enableReceiveMove(true);
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

    void CameraComponent::enable()
    {
        if (isActive())
        {
            getOwnerLevel().getGraphicsWorld().setMainCamera(camera_);
        }
        else
        {
            setToMainCamera_ = true;
        }
    }

    void CameraComponent::disable()
    {
        if (isActive() && getOwnerLevel().getGraphicsWorld().getMainCamera() == camera_)
        {
            getOwnerLevel().getGraphicsWorld().setMainCamera(nullptr);
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
        if (setToMainCamera_)
        {
            enable();
            setToMainCamera_ = false;
        }
    }

    void CameraComponent::onDeactivate()
    {
        disable();
        getOwnerLevel().getGraphicsWorld().removeCamera(camera_);
    }
}