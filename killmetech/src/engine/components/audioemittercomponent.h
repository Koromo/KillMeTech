#ifndef _KILLME_AUDIOEMITTERCOMPONENT_H_
#define _KILLME_AUDIOEMITTERCOMPONENT_H_

#include "transformcomponent.h"
#include "../../audio/audio3d.h"
#include "../../processes/process.h"
#include "../../resources/resource.h"
#include <memory>

namespace killme
{
    class SourceVoice;
    class AudioClip;

    /** Whether emit audio in 3D audio (world) or local audio */
    enum class AudioType
    {
        local,
        world
    };

    /** The audio emitter component adds function of emits audio into an actor */
    class AudioEmitterComponent : public TransformComponent
    {
    private:
        AudioType type_;
        std::shared_ptr<SourceVoice> voice_;
        EmitterParams params_;
        Process process_;

    public:
        /** Construct */
        AudioEmitterComponent(AudioType type, const Resource<AudioClip>& clip);

        /** If audio is pausing, restart. Else if audio is not queued, play at once. Otherwise, nothing to do. */
        void play();

        /** Start audio loop (numLoops < AUDIO_LOOP_INFINITE)*/
        void start(size_t numLoops);

        /** Stop the audio */
        void stop();

        /** Pause the audio */
        void pause();

        /** Return true if the audio is playing now */
        bool isPlaying() const;

        void onAttached();
        void onDettached();

    private:
        void tickAudioWorld();
    };
}

#endif