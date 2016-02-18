#ifndef _KILLME_CAMERA_H_
#define _KILLME_CAMERA_H_

#include "../renderer/renderstate.h"
#include "../core/math/math.h"
#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include "../core/math/matrix44.h"

namespace killme
{
    /** The camera */
    class Camera
    {
    private:
        float fovX_;
        float aspect_;
        float nearZ_;
        float farZ_;
        Viewport viewport_;
        Vector3 position_;
        Quaternion orientation_;

    public:
        /** Constructs */
        Camera(const Viewport& vp)
            : fovX_(radian(60))
            , aspect_()
            , nearZ_(0.1f)
            , farZ_(1000)
            , viewport_(vp)
            , position_()
            , orientation_()
        {
            aspect_ = viewport_.width / viewport_.height;
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

        /** Transform modifiers */
        void setPosition(const Vector3& pos) { position_ = pos; }
        void setOrientation(const Quaternion& q) { orientation_ = q; }
        Matrix44 getViewMatrix() const { return inverse(makeTransformMatrix({1, 1, 1}, orientation_, position_)); }
    };
}

#endif
