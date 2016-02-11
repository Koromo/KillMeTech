#ifndef _KILLME_CAMERA_H_
#define _KILLME_CAMERA_H_

#include "sceneentity.h"
#include "scenevisitor.h"
#include "../renderer/renderstate.h"
#include "../renderer/rendersystem.h"
#include "../core/math/math.h"
#include "../core/math/matrix44.h"

namespace killme
{
    /** The camera */
    class Camera : public SceneEntity, public std::enable_shared_from_this<Camera>
    {
    private:
        float fovX_;
        float aspect_;
        float nearZ_;
        float farZ_;
        Viewport viewport_;

    public:
        /** Constructs */
        Camera()
            : fovX_(radian(60))
            , aspect_(16 / 9.0f)
            , nearZ_(0.1f)
            , farZ_(1000)
            , viewport_(renderSystem.getDefaultViewport())
        {
        }

        /** Sets a viewport */
        void setViewport(const Viewport& vp) { viewport_ = vp; }

        /** Returns the viewport */
        Viewport getViewport() const { return viewport_; }

        /** Updates the projection */
        void setFovX(float rad) { fovX_ = rad; }
        void setAspectRate(float aspect) { aspect_ = aspect; }
        void setNearZ(float z) { nearZ_ = z; }
        void setFarZ(float z) { farZ_ = z; }

        /** Returns the projection matrix */
        Matrix44 getProjectionMatrix() const { return makeProjectionMatrix(fovX_, aspect_, nearZ_, farZ_); }

        bool accept(SceneVisitor& v) { return v(lockOwner(), shared_from_this()); }
    };
}

#endif
