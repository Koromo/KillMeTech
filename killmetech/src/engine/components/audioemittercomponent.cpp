#include "audioemittercomponent.h"
#include "../audio.h"
#include "../resources.h"
#include "../processes.h"
#include "../../audio/sourcevoice.h"
#include "../../audio/audioclip.h"
#include "../../audio/audio3d.h"

namespace killme
{
    AudioEmitterComponent::AudioEmitterComponent(const std::string& path)
        : voice_(Audio::createSourceVoice(Resources::load<AudioClip>(path)))
        , process_()
    {
    }

    void AudioEmitterComponent::play()
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

    void AudioEmitterComponent::start(size_t numLoops)
    {
        voice_->stop();
        voice_->flush();
        voice_->submit(numLoops);
        voice_->start();
    }

    void AudioEmitterComponent::stop()
    {
        voice_->stop();
        voice_->flush();
    }

    void AudioEmitterComponent::pause()
    {
        voice_->stop();
    }

    bool AudioEmitterComponent::isPlaying() const
    {
        return voice_->isPlaying();
    }

    void AudioEmitterComponent::onAttached()
    {
        TransformComponent::onAttached();
        process_ = Processes::start([&] { tickAudioWorld(); }, PROCESS_PRIORITY_AUDIO_EMITTER);
    }

    void AudioEmitterComponent::onDettached()
    {
        TransformComponent::onDettached();
        process_.kill();
    }

    void AudioEmitterComponent::tickAudioWorld()
    {
        EmitterParams params;
        params.position = getWorldPosition();
        params.orientation = getWorldOrientation();
        params.voice = voice_;
        Audio::apply3DEmission(params);
    }
}