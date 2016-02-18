#ifndef _KILLME_AUDIOEMITTERCOMPONENT_H_
#define _KILLME_AUDIOEMITTERCOMPONENT_H_

#include "transformcomponent.h"
#include "processes.h"
#include "resources.h"
#include "audio.h"
#include "../processes/process.h"
#include "../audio/sourcevoice.h"
#include "../audio/audio3d.h"
#include <memory>
#include <string>

namespace killme
{
    class AudioClip;

    /** The audio emitter component adds function of emits audio into an actor */
    class AudioEmitterComponent : public TransformComponent
    {
    private:
        std::shared_ptr<SourceVoice> voice_;
        Process process_;

    public:
        /** Constructs */
        explicit AudioEmitterComponent(const std::string& path)
            : voice_(Audio::createSourceVoice(Resources::load<AudioClip>(path)))
            , process_()
        {
        }

        /** If audio is pausing, restart. Else if audio is not queued, play at once. Otherwise, nothing to do. */
        void play()
        {
            if (voice_->isPlaying())
            {
                return;
            }
            if (voice_->getNumQueued() == 0)
            {
                voice_->submit(1);
            }
            voice_->start();
        }

        /** Starts audio loop */
        void start(size_t numLoops)
        {
            voice_->stop();
            voice_->flush();
            voice_->submit(numLoops);
            voice_->start();
        }

        /** Stops the audio */
        void stop()
        {
            voice_->stop();
            voice_->flush();
        }

        /** Pauses the audio */
        void pause()
        {
            voice_->stop();
        }

        /** Returns true if the audio is playing now */
        bool isPlaying() const
        {
            voice_->isPlaying();
        }

        void onAttached()
        {
            TransformComponent::onAttached();
            process_ = Processes::start([&] { tickAudioWorld(); }, PROCESS_PRIORITY_AUDIO_EMITTER);
        }

        void onDettached()
        {
            TransformComponent::onDettached();
            process_.kill();
        }

    private:
        void tickAudioWorld()
        {
            EmitterParams params;
            params.position = getWorldPosition();
            params.orientation = getWorldOrientation();
            params.voice = voice_;
            Audio::apply3DEmission(params);
        }
    };
}

#endif