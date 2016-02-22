#ifndef _KILLME_LIGHTCOMPONENT_H_
#define _KILLME_LIGHTCOMPONENT_H_

#include "transformcomponent.h"
#include "../../processes/process.h"
#include <memory>

namespace killme
{
    class Light;
    class Color;
    class Vector3;
    class Quaternion;

    /** The camera component adds a light source into an actor */
    class LightComponent : public TransformComponent
    {
    private:
        std::shared_ptr<Light> light_;
        Process process_;

    public:
        /** Construct */
        LightComponent();

        /** Property modifier */
        void setColor(const Color& c);

        void onAttached();
        void onDettached();

    private:
        void tickScene();
    };
}

#endif