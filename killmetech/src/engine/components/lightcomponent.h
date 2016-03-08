#ifndef _KILLME_LIGHTCOMPONENT_H_
#define _KILLME_LIGHTCOMPONENT_H_

#include "transformcomponent.h"
#include <memory>

namespace killme
{
    class Light;
    class Color;

    /** The light component defines a light source into an actor */
    class LightComponent : public TransformComponent
    {
        KILLME_COMPONENT_DEFINE(LightComponent)

    private:
        std::shared_ptr<Light> light_;

    public:
        /** Construct */
        LightComponent();

        /** Property modifier */
        void setColor(const Color& c);

        void onTranslated();
        void onRotated();

        void onActivate();
        void onDeactivate();
    };
}

#endif