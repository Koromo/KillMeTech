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