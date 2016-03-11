#include "listenercomponent.h"
#include "../runtime.h"
#include "../level.h"
#include "../../audio/audioworld.h"

namespace killme
{
    ListenerComponent::ListenerComponent()
        : listener_(std::make_shared<AudioListener>())
        , isMainListener_(false)
    {
        setMoveRecievable(true);
    }

    void ListenerComponent::setEnable(bool enable)
    {
        isMainListener_ = enable;
        if (isActive())
        {
            if (enable)
            {
                getOwnerLevel().getAudioWorld().setMainListener(listener_);
            }
            else if (getOwnerLevel().getAudioWorld().getMainListener() == listener_)
            {
                getOwnerLevel().getAudioWorld().setMainListener(nullptr);
            }
        }
    }

    void ListenerComponent::onTranslated()
    {
        listener_->position = getWorldPosition();
        listener_->velocity = getWorldVelocity(*this, runtime.getDeltaTime());
    }

    void ListenerComponent::onRotated()
    {
        listener_->orientation = getWorldOrientation();
    }

    void ListenerComponent::onActivate()
    {
        getOwnerLevel().getAudioWorld().addListener(listener_);
        if (isMainListener_)
        {
            getOwnerLevel().getAudioWorld().setMainListener(listener_);
        }
    }

    void ListenerComponent::onDeactivate()
    {
        setEnable(false);
        getOwnerLevel().getAudioWorld().removeListener(listener_);
    }
}