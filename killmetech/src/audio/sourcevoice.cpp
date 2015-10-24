#include "sourcevoice.h"
#include "audioclip.h"
#include "../core/exception.h"
#include <cassert>

namespace killme
{
    void CALLBACK SourceVoice::VoiceCallback::OnBufferEnd(void* context)
    {
        // Down the audio playing flag
        const auto p = static_cast<SourceVoice*>(context);
        assert(p && "Source buffer context is not SourceVoice.");
        p->isPlaying_ = false;
    }

    void CALLBACK SourceVoice::VoiceCallback::OnVoiceError(void*, HRESULT)
    {
        throw XAudioException("Any error occurred in source buffer.");
    }

    SourceVoice::SourceVoice(const std::weak_ptr<IXAudio2>& xAudio, const std::shared_ptr<const AudioClip>& clip)
        : xAudio_(xAudio)
        , sourceVoice_()
        , clip_(clip)
        , callBack_()
        , isPlaying_(false)
    {
        const auto format = clip_->getFormat();
        IXAudio2SourceVoice* voice;
        enforce<XAudioException>(
            SUCCEEDED(xAudio.lock()->CreateSourceVoice(&voice, &format, 0, 2, &callBack_)),
            "Failed to create IXAudio2SourceVoice."
            );
        sourceVoice_ = makeVoiceUnique(voice);
    }

    SourceVoice::~SourceVoice()
    {
        if (xAudio_.expired())
        {
            sourceVoice_.release();
        }
    }

    void SourceVoice::play()
    {
        // Submit audio data if audio buffer is empty
        XAUDIO2_VOICE_STATE state;
        sourceVoice_->GetState(&state);
        if (state.BuffersQueued == 0)
        {
            XAUDIO2_BUFFER buffer;
            ZeroMemory(&buffer, sizeof(buffer));
            buffer.Flags = XAUDIO2_END_OF_STREAM;
            buffer.AudioBytes = clip_->getSize();
            buffer.pAudioData = clip_->getData();
            buffer.LoopBegin = XAUDIO2_NO_LOOP_REGION;
            buffer.pContext = this;

            enforce<XAudioException>(
                SUCCEEDED(sourceVoice_->SubmitSourceBuffer(&buffer)),
                "Failed to submit audio buffer."
                );
        }

        // Start audio
        enforce<XAudioException>(
            SUCCEEDED(sourceVoice_->Start(0)),
            "Failed to start source voice."
            );
        isPlaying_ = true;
    }

    void SourceVoice::stop()
    {
        // Stop audio and flush buffer
        pause();
        enforce<XAudioException>(
            SUCCEEDED(sourceVoice_->FlushSourceBuffers()),
            "Failed to flush source buffer."
            );
    }

    void SourceVoice::pause()
    {
        enforce<XAudioException>(
            SUCCEEDED(sourceVoice_->Stop()),
            "Failed to stop source voice."
            );
        isPlaying_ = false;
    }

    bool SourceVoice::isPlaying() const
    {
        return isPlaying_;
    }
}