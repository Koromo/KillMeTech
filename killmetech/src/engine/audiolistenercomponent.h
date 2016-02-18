#ifndef _KILLME_AUDIOLISTENERCOMPONENT_H_
#define _KILLME_AUDIOLISTENERCOMPONENT_H_

#include "transformcomponent.h"
#include "processes.h"
#include "audio.h"
#include "../audio/audio3d.h"
#include "../processes/process.h"

namespace killme
{
    /** The audio listener component adds function of listening audio into an actor */
    class AudioListenerComponent : public TransformComponent
    {
    private:
        Process process_;

    public:
        void onAttached()
        {
            TransformComponent::onAttached();
            process_ = Processes::start([&] { tickAudioWorld(); }, PROCESS_PRIORITY_AUDIO_LISTENER);
        }

        void onDettached()
        {
            TransformComponent::onDettached();
            process_.kill();
        }

    private:
        void tickAudioWorld()
        {
            ListenerParams params;
            params.position = getWorldPosition();
            params.orientation = getWorldOrientation();
            Audio::set3DListener(params);
        }
    };
}

#endif