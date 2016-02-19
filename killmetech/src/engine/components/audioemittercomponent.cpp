#include "audioemittercomponent.h"
#include "../audio.h"
#include "../resources.h"
#include "../processes.h"
#include "../runtime.h"
#include "../../audio/sourcevoice.h"
#include "../../audio/audioclip.h"

namespace killme
{
    AudioEmitterComponent::AudioEmitterComponent(const std::string& path)
        : voice_(Audio::createSourceVoice(Resources::load<AudioClip>(path)))
        , params_()
        , process_()
    {
        params_.voice = voice_;
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
        const auto dt_s = RunTime::getDeltaTime();

        const auto nowPos = getWorldPosition();
        const auto prePos = params_.position;

        params_.position = getWorldPosition();
        params_.orientation = getWorldOrientation();
        params_.velocity = (nowPos - prePos) / dt_s;
        params_.voice = voice_;

        Audio::apply3DEmission(params_);
    }
}