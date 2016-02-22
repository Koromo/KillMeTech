#include "lightcomponent.h"
#include "../graphics.h"
#include "../processes.h"
#include "../../scene/light.h"

namespace killme
{
    LightComponent::LightComponent()
        : light_(std::make_shared<Light>())
        , process_()
    {
    }

    void LightComponent::setColor(const Color& c)
    {
        light_->setColor(c);
    }

    void LightComponent::onAttached()
    {
        TransformComponent::onAttached();
        Graphics::addLight(light_);
        process_ = Processes::start([&] { tickScene(); }, PROCESS_PRIORITY_SCENE);
    }

    void LightComponent::onDettached()
    {
        TransformComponent::onDettached();
        Graphics::removeLight(light_);
        process_.kill();
    }

    void LightComponent::tickScene()
    {
        light_->setPosition(getWorldPosition());
        light_->setOrientation(getWorldOrientation());
    }
}