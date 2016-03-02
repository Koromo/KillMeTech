#ifndef _KILLME_CAMERACOMPONENT_H_
#define _KILLME_CAMERACOMPONENT_H_

#include "transformcomponent.h"
#include "../../processes/process.h"
#include <memory>

namespace killme
{
    class Camera;
    struct Viewport;

    /** The camera component adds definition of virtual camera for render scene into an actor */
    class CameraComponent : public TransformComponent
    {
    private:
        std::shared_ptr<Camera> camera_;
        Process process_;

    public:
        /** Construct */
        CameraComponent();

        /** Set viewport */
        void setViewport(const Viewport& vp);

        /** Projection modifiers */
        void setFovX(float rad);
        void setAspectRate(float aspect); /// NOTE: w/h
        void setNearZ(float z);
        void setFarZ(float z);

        void onAttached();
        void onDettached();

    private:
        void tickScene();
    };
}

#endif