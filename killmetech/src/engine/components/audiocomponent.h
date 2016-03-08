#ifndef _KILLME_AUDIOCOMPONENT_H_
#define _KILLME_AUDIOCOMPONENT_H_

#include "transformcomponent.h"
#include "../../resources/resource.h"
#include <memory>

namespace killme
{
    class AudioEmitter;
    class AudioClip;
    class SourceVoice;

    /** The audio component defines emitting audio into an actor */
    class AudioComponent : public TransformComponent
    {
        KILLME_COMPONENT_DEFINE(AudioComponent)

    private:
        std::shared_ptr<AudioEmitter> emitter_;
        std::shared_ptr<SourceVoice> localVoice_;

    public:
        /** Construct */
        explicit AudioComponent(const Resource<AudioClip>& clip);

        /** If audio is pausing, restart. Else if audio is not queued, play at once. Otherwise, nothing to do. */
        void play(bool local = false);

        /** Start audio loop (numLoops <= AUDIO_LOOP_INFINITE)*/
        void start(size_t numLoops, bool local = false);

        /** Stop the audio */
        void stop(bool local = false);

        /** Pause the audio */
        void pause(bool local = false);

        /** Return true if the audio is playing now */
        bool isPlaying(bool local = false) const;

        void onTranslated();
        void onRotated();

        void onActivate();
        void onDeactivate();
    };
}

#endif