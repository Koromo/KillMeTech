#ifndef _KILLME_CAMERACOMPONENT_H_
#define _KILLME_CAMERACOMPONENT_H_

#include "transformcomponent.h"
#include "graphics.h"
#include "processes.h"
#include "../processes/process.h"
#include "../scene/camera.h"
#include <memory>

namespace killme
{
    /** The camera component adds definition of virtual camera for render scene into an actor */
    class CameraComponent : public TransformComponent
    {
    private:
        std::shared_ptr<Camera> camera_;
        Process process_;

    public:
        /** Constructs */
        CameraComponent()
            : camera_(std::make_shared<Camera>(Graphics::getClientViewport()))
            , process_()
        {
        }

        void onAttached()
        {
            TransformComponent::onAttached();
            process_ = Processes::start([&] { tickScene(); }, PROCESS_PRIORITY_SCENE);
            Graphics::setMainCamera(camera_);
        }

        void onDettached()
        {
            TransformComponent::onDettached();
            Graphics::setMainCamera(nullptr);
            process_.kill();
        }

    private:
        void tickScene()
        {
            camera_->setPosition(getWorldPosition());
            camera_->setOrientation(getWorldOrientation());
        }
    };
}

#endif