#include "sourcevoice.h"
#include "audioclip.h"
#include "audiomanager.h"
#include "../core/exception.h"
#include <cassert>

namespace killme
{
    const int AUDIO_LOOP_INFINITE = XAUDIO2_LOOP_INFINITE + 1;

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

    SourceVoice::SourceVoice(IXAudio2* xAudio, const Resource<AudioClip>& clip)
        : sourceVoice_()
        , clip_(clip)
        , callBack_()
        , isPlaying_(false)
    {
        const auto format = clip_.access()->getFormat();
        IXAudio2SourceVoice* voice;
        enforce<XAudioException>(
            SUCCEEDED(xAudio->CreateSourceVoice(&voice, &format, 0, 2, &callBack_)),
            "Failed to create IXAudio2SourceVoice."
            );
        sourceVoice_ = makeVoiceUnique(voice);
    }

    SourceVoice::~SourceVoice()
    {
        if (!audioManager.isActive())
        {
            sourceVoice_.release();
        }
    }

    void SourceVoice::play(size_t numLoops)
    {
        if (isPlaying_)
        {
            return;
        }

        if (numLoops > AUDIO_LOOP_INFINITE)
        {
            numLoops = AUDIO_LOOP_INFINITE;
        }

        // Submit the audio data into the buffer
        XAUDIO2_BUFFER buffer;
        ZeroMemory(&buffer, sizeof(buffer));
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.AudioBytes = static_cast<UINT32>(clip_.access()->getSize());
        buffer.pAudioData = clip_.access()->getData();
        buffer.PlayBegin = 0;
        buffer.PlayLength = 0;
        buffer.LoopBegin = 0;
        buffer.LoopLength = 0;
        buffer.LoopCount = numLoops - 1;
        buffer.pContext = this;

        enforce<XAudioException>(
            SUCCEEDED(sourceVoice_->SubmitSourceBuffer(&buffer)),
            "Failed to submit audio buffer."
            );

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

    void SourceVoice::applyFrequencyRatio(float ratio)
    {
        enforce<XAudioException>(
            SUCCEEDED(sourceVoice_->SetFrequencyRatio(ratio)),
            "Failed to set the frequency ratio.");
    }

    void SourceVoice::applyOutputMatrix(size_t numSrcChannels, size_t numDestChannels, const float* levelMatrix)
    {
        enforce<XAudioException>(
            SUCCEEDED(sourceVoice_->SetOutputMatrix(nullptr, numSrcChannels, numDestChannels, levelMatrix)),
            "Failed to set the output matrix.");
    }
}