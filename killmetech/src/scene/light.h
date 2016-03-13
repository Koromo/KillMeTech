#ifndef _KILLME_LIGHT_H_
#define _KILLME_LIGHT_H_

#include "../core/math/vector3.h"
#include "../core/math/quaternion.h"
#include "../core/math/color.h"

namespace killme
{
    /** Light type */
    enum class LightType
    {
        directional,
        point
    };

    /** Light */
    class Light
    {
    private:
        LightType type_;
        Vector3 position_;
        Quaternion orientation_;
        Color color_;
        float attRange_;
        float attConstant_;
        float attLiner_;
        float attQuadratic_;

    public:
        /** Construct */
        explicit Light(LightType type)
            : type_(type)
            , position_()
            , orientation_()
            , color_(0.2f, 0.2f, 0.2f, 1)
            , attRange_(100)
            , attConstant_(1)
            , attLiner_(0)
            , attQuadratic_(0)
        {
        }

        /** Return the light type */
        LightType getType() const { return type_; }

        /** Color modifier (Directional, Point) */
        void setColor(const Color& c) { color_ = c; }
        Color getColor() const { return color_; }

        /** Position modifier (Point) */
        void setPosition(const Vector3& pos) { position_ = pos; }
        Vector3 getPosition() const { return position_; }

        /** Direction modifier (Directional) */
        void setOrientation(const Quaternion& q) { orientation_ = q; }

        /** Attenuation modifier (Point) */
        void setAttenuation(float range, float constant, float liner, float quadratic)
        {
            attRange_ = range;
            attConstant_ = constant;
            attLiner_ = liner;
            attQuadratic_ = quadratic;
        }
        float getAttenuationRange() const { return attRange_; }
        float getAttenuationConstant() const { return attConstant_; }
        float getAttenuationLiner() const { return attLiner_; }
        float getAttenuationQuadratic() const { return attQuadratic_; }

        /** Return the front direction (Directional) */
        Vector3 getDirection() const { return normalize(orientation_ * Vector3::UNIT_Z); }
    };
}

#endif