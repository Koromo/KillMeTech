#ifndef _KILLME_POINTLIGHTCOMPONENT_H_
#define _KILLME_POINTLIGHTCOMPONENT_H_

#include "transformcomponent.h"
#include <memory>

namespace killme
{
    class Light;
    class Color;
    enum class LightType;

    /** The PointLightComponents defines a point light source into an actor */
    class PointLightComponent : public TransformComponent
    {
        KILLME_COMPONENT_DEFINE_BEGIN(PointLightComponent)
        KILLME_COMPONENT_DEFINE_END

    private:
        std::shared_ptr<Light> light_;

    public:
        /** Construct */
        PointLightComponent();

        /** Property modifier */
        void setColor(const Color& c);
        void setAttenuation(float range, float constant, float liner, float quadratic);

    private:
        void onTranslated();

        void onActivate();
        void onDeactivate();
    };
}

#endif