#include "cameracomponent.h"
#include "../graphics.h"
#include "../processes.h"
#include "../../scene/camera.h"

namespace killme
{
    CameraComponent::CameraComponent()
        : camera_(std::make_shared<Camera>(Graphics::getClientViewport()))
        , process_()
    {
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

    void CameraComponent::onAttached()
    {
        TransformComponent::onAttached();
        process_ = Processes::start([&] { tickScene(); }, PROCESS_PRIORITY_SCENE);
        Graphics::setMainCamera(camera_);
    }

    void CameraComponent::onDettached()
    {
        TransformComponent::onDettached();
        Graphics::setMainCamera(nullptr);
        process_.kill();
    }

    void CameraComponent::tickScene()
    {
        camera_->setPosition(getWorldPosition());
        camera_->setOrientation(getWorldOrientation());
    }
}