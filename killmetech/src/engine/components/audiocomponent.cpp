#include "audiocomponent.h"
#include "../runtime.h"
#include "../level.h"
#include "../audiosystem.h"
#include "../../audio/audioworld.h"
#include "../../audio/sourcevoice.h"

namespace killme
{
    AudioComponent::AudioComponent(const Resource<AudioClip>& clip)
        : emitter_()
        , localVoice_(audioSystem.createSourceVoice(clip))
    {
        emitter_->voice = audioSystem.createSourceVoice(clip);
        setMoveRecievable(true);
    }

    void AudioComponent::play(bool local)
    {
        const auto voice = local ? localVoice_ : emitter_->voice;
        if (voice->isPlaying())
        {
            return;
        }
        if (voice->getNumQueued() == 0)
        {
            voice->submit(1);
        }
        voice->start();
    }

    void AudioComponent::start(size_t numLoops, bool local)
    {
        const auto voice = local ? localVoice_ : emitter_->voice;
        voice->stop();
        voice->flush();
        voice->submit(numLoops);
        voice->start();
    }

    void AudioComponent::stop(bool local)
    {
        const auto voice = local ? localVoice_ : emitter_->voice;
        voice->stop();
        voice->flush();
    }

    void AudioComponent::pause(bool local)
    {
        const auto voice = local ? localVoice_ : emitter_->voice;
        voice->stop();
    }

    bool AudioComponent::isPlaying(bool local) const
    {
        const auto voice = local ? localVoice_ : emitter_->voice;
        return voice->isPlaying();
    }

    void AudioComponent::onTranslated()
    {
        emitter_->position = getWorldPosition();
        emitter_->velocity = getWorldVelocity(*this, runtime.getDeltaTime());
    }

    void AudioComponent::onRotated()
    {
        emitter_->orientation = getWorldOrientation();
    }

    void AudioComponent::onActivate()
    {
        getOwnerLevel().getAudioWorld().addEmitter(emitter_);
    }

    void AudioComponent::onDeactivate()
    {
        getOwnerLevel().getAudioWorld().removeEmitter(emitter_);
    }
}