#ifndef _KILLME_SOURCEVOICE_H_
#define _KILLME_SOURCEVOICE_H_

#include "xaudiosupport.h"
#include "../resources/resource.h"
#include <xaudio2.h>
#include <memory>

namespace killme
{
    class AudioClip;

    extern const int AUDIO_LOOP_INFINITE;

    /** Transmit the audio data to the audio device */
    class SourceVoice
    {
    private:
        struct VoiceCallback : public IXAudio2VoiceCallback
        {
            void CALLBACK OnBufferEnd(void* context);
            void CALLBACK OnBufferStart(void*) {}
            void CALLBACK OnLoopEnd(void*) {}
            void CALLBACK OnStreamEnd() {}
            void CALLBACK OnVoiceError(void* context, HRESULT hr);
            void CALLBACK OnVoiceProcessingPassEnd() {}
            void CALLBACK OnVoiceProcessingPassStart(unsigned) {}
        };

        std::weak_ptr<IXAudio2> xAudio_;
        VoiceUniquePtr<IXAudio2SourceVoice> sourceVoice_;
        Resource<AudioClip> clip_;
        VoiceCallback callBack_;
        bool isPlaying_;

    public:
        /** Constructs with an audio clip */
        SourceVoice(const std::weak_ptr<IXAudio2>& xAudio, const Resource<AudioClip>& clip);

        /** Destructs */
        ~SourceVoice();

        /** Submits the audio buffer into buffer queue */
        void submit(size_t numLoops);

        /** Returns count of queued buffers */
        size_t getNumQueued();

        /** Start voice */
        void start();

        /** Stops audio */
        void stop();

        /** Flushs audio */
        void flush();

        /** Returns true if audio is playing now */
        bool isPlaying() const;

        /** Applies the frequency ratio */
        void applyFrequencyRatio(float ratio);

        /** Applies the output matrix */
        void applyOutputMatrix(size_t numSrcChannels, size_t numDestChannels, const float* levelMatrix);
    };
}

#endif