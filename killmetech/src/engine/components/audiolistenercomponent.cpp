#include "audiolistenercomponent.h"
#include "../processes.h"
#include "../audio.h"
#include "../runtime.h"

namespace killme
{
    void AudioListenerComponent::onAttached()
    {
        TransformComponent::onAttached();
        process_ = Processes::start([&] { tickAudioWorld(); }, PROCESS_PRIORITY_AUDIO_LISTENER);
    }

    void AudioListenerComponent::onDettached()
    {
        TransformComponent::onDettached();
        process_.kill();
    }

    void AudioListenerComponent::tickAudioWorld()
    {
        const auto dt_s = RunTime::getDeltaTime();

        const auto nowPos = getWorldPosition();
        const auto prePos = params_.position;

        params_.position = nowPos;
        params_.orientation = getWorldOrientation();
        params_.velocity = (nowPos - prePos) / dt_s;

        Audio::set3DListener(params_);
    }
}