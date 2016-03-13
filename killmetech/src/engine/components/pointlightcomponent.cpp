#include "pointlightcomponent.h"
#include "../level.h"
#include "../../scene/light.h"
#include "../../scene/scene.h"

namespace killme
{
    PointLightComponent::PointLightComponent()
        : light_(std::make_shared<Light>(LightType::point))
    {
        setMoveRecievable(true);
    }

    void PointLightComponent::setColor(const Color& c)
    {
        light_->setColor(c);
    }

    void PointLightComponent::setAttenuation(float range, float constant, float liner, float quadratic)
    {
        light_->setAttenuation(range, constant, liner, quadratic);
    }

    void PointLightComponent::onTranslated()
    {
        light_->setPosition(getWorldPosition());
    }

    void PointLightComponent::onActivate()
    {
        getOwnerLevel().getGraphicsWorld().addLight(light_);
    }

    void PointLightComponent::onDeactivate()
    {
        getOwnerLevel().getGraphicsWorld().removeLight(light_);
    }
}