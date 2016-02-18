#include "audiolistenercomponent.h"
#include "../processes.h"
#include "../audio.h"
#include "../../audio/audio3d.h"

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
        ListenerParams params;
        params.position = getWorldPosition();
        params.orientation = getWorldOrientation();
        Audio::set3DListener(params);
    }
}