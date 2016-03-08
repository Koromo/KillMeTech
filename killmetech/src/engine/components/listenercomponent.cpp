#include "listenercomponent.h"
#include "../runtime.h"
#include "../level.h"
#include "../../audio/audioworld.h"

namespace killme
{
    ListenerComponent::ListenerComponent()
        : listener_(std::make_shared<AudioListener>())
        , setToMainListener_(false)
    {
        enableReceiveMove(true);
    }

    void ListenerComponent::enable()
    {
        if (isActive())
        {
            getOwnerLevel().getAudioWorld().setMainListener(listener_);
        }
        else
        {
            setToMainListener_ = true;
        }
    }

    void ListenerComponent::disable()
    {
        if (getOwnerLevel().getAudioWorld().getMainListener() == listener_)
        {
            getOwnerLevel().getAudioWorld().setMainListener(nullptr);
        }
    }

    void ListenerComponent::onTranslated()
    {
        listener_->position = getWorldPosition();
        listener_->velocity = getWorldVelocity(*this, runTime.getDeltaTime());
    }

    void ListenerComponent::onRotated()
    {
        listener_->orientation = getWorldOrientation();
    }

    void ListenerComponent::onActivate()
    {
        getOwnerLevel().getAudioWorld().addListener(listener_);
        if (setToMainListener_)
        {
            enable();
            setToMainListener_ = false;
        }
    }

    void ListenerComponent::onDeactivate()
    {
        disable();
        getOwnerLevel().getAudioWorld().removeListener(listener_);
    }
}