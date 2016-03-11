#include "lightcomponent.h"
#include "../level.h"
#include "../../scene/light.h"
#include "../../scene/scene.h"

namespace killme
{
    LightComponent::LightComponent()
        : light_(std::make_shared<Light>())
    {
        setMoveRecievable(true);
    }

    void LightComponent::setColor(const Color& c)
    {
        light_->setColor(c);
    }

    void LightComponent::onTranslated()
    {
        light_->setPosition(getWorldPosition());
    }

    void LightComponent::onRotated()
    {
        light_->setOrientation(getWorldOrientation());
    }

    void LightComponent::onActivate()
    {
        getOwnerLevel().getGraphicsWorld().addLight(light_);
    }

    void LightComponent::onDeactivate()
    {
        getOwnerLevel().getGraphicsWorld().removeLight(light_);
    }
}