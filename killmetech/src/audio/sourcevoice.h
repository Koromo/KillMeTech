#ifndef _KILLME_SOURCEVOICE_H_
#define _KILLME_SOURCEVOICE_H_

#include "xaudiosupport.h"
#include <xaudio2.h>
#include <memory>

namespace killme
{
    class AudioClip;

    /** Transmit audio data to audio device for playing audio */
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
        std::shared_ptr<const AudioClip> clip_;
        VoiceCallback callBack_;
        bool isPlaying_;

    public:
        /** Construct with a audio clip */
        SourceVoice(const std::weak_ptr<IXAudio2>& xAudio, const std::shared_ptr<const AudioClip>& clip);

        /** Destruct */
        ~SourceVoice();

        /** Play audio at once */
        void play();

        /** Stop audio */
        void stop();

        /** Pause audio */
        void pause();

        /** Returns true if audio is playing now */
        bool isPlaying() const;
    };
}

#endif