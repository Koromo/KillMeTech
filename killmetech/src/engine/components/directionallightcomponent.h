#ifndef _KILLME_DIRECTIONALLIGHTCOMPONENT_H_
#define _KILLME_DIRECTIONALLIGHTCOMPONENT_H_

#include "transformcomponent.h"
#include <memory>

namespace killme
{
    class Light;
    class Color;
    enum class LightType;

    /** The DirectionalLightComponents defines a directional light source into an actor */
    class DirectionalLightComponent : public TransformComponent
    {
        KILLME_COMPONENT_DEFINE_BEGIN(DirectionalLightComponent)
        KILLME_COMPONENT_DEFINE_END

    private:
        std::shared_ptr<Light> light_;

    public:
        /** Construct */
        DirectionalLightComponent();

        /** Property modifier */
        void setColor(const Color& c);

    private:
        void onRotated();

        void onActivate();
        void onDeactivate();
    };
}

#endif