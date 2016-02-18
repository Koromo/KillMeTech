#ifndef _KILLME_CAMERACOMPONENT_H_
#define _KILLME_CAMERACOMPONENT_H_

#include "transformcomponent.h"
#include "../../processes/process.h"
#include <memory>

namespace killme
{
    class Camera;

    /** The camera component adds definition of virtual camera for render scene into an actor */
    class CameraComponent : public TransformComponent
    {
    private:
        std::shared_ptr<Camera> camera_;
        Process process_;

    public:
        /** Constructs */
        CameraComponent();

        void onAttached();
        void onDettached();

    private:
        void tickScene();
    };
}

#endif