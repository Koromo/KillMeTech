#include "directionallightcomponent.h"
#include "../level.h"
#include "../../scene/light.h"
#include "../../scene/scene.h"

namespace killme
{
    DirectionalLightComponent::DirectionalLightComponent()
        : light_(std::make_shared<Light>(LightType::directional))
    {
        setMoveRecievable(true);
    }

    void DirectionalLightComponent::setColor(const Color& c)
    {
        light_->setColor(c);
    }

    void DirectionalLightComponent::onRotated()
    {
        light_->setOrientation(getWorldOrientation());
    }

    void DirectionalLightComponent::onActivate()
    {
        getOwnerLevel().getGraphicsWorld().addLight(light_);
    }

    void DirectionalLightComponent::onDeactivate()
    {
        getOwnerLevel().getGraphicsWorld().removeLight(light_);
    }
}