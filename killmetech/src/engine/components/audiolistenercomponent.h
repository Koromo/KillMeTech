#ifndef _KILLME_AUDIOLISTENERCOMPONENT_H_
#define _KILLME_AUDIOLISTENERCOMPONENT_H_

#include "transformcomponent.h"
#include "../../processes/process.h"

namespace killme
{
    /** The audio listener component adds function of listening audio into an actor */
    class AudioListenerComponent : public TransformComponent
    {
    private:
        Process process_;

    public:
        void onAttached();
        void onDettached();

    private:
        void tickAudioWorld();
    };
}

#endif