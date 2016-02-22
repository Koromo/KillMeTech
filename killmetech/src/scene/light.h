#ifndef _KILLME_LIGHT_H_
#define _KILLME_LIGHT_H_

#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include "../core/math/color.h"

namespace killme
{
    /** Light */
    class Light
    {
    private:
        Vector3 position_;
        Quaternion orientation_;
        Color color_;

    public:
        /** Construct */
        Light()
            : position_()
            , orientation_()
            , color_(0.2f, 0.2f, 0.2f, 1)
        {
        }

        /** Color modifier */
        void setColor(const Color& c) { color_ = c; }
        Color getColor() const { return color_; }

        /** Transform modifiers */
        void setPosition(const Vector3& pos) { position_ = pos; }
        void setOrientation(const Quaternion& q) { orientation_ = q; }

        /** Return front direction */
        Vector3 getFront() const { return normalize(orientation_ * Vector3::UNIT_Z); }
    };
}

#endif