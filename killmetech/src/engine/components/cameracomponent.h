#ifndef _KILLME_CAMERACOMPONENT_H_
#define _KILLME_CAMERACOMPONENT_H_

#include "transformcomponent.h"
#include <memory>

namespace killme
{
    class Camera;
    struct Viewport;

    /** The camera component defines virtual camera for render scene into an actor */
    class CameraComponent : public TransformComponent
    {
        KILLME_COMPONENT_DEFINE_BEGIN(CameraComponent)
        KILLME_COMPONENT_DEFINE_END

    private:
        std::shared_ptr<Camera> camera_;
        bool isMainCamera_;

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

        /** If true, set this camera as the main camera */
        void setEnable(bool enable);

    private:
        void onTranslated();
        void onRotated();

        void onActivate();
        void onDeactivate();
    };
}

#endif