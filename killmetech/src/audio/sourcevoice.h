#ifndef _KILLME_SOURCEVOICE_H_
#define _KILLME_SOURCEVOICE_H_

#include "xaudiosupport.h"
#include <xaudio2.h>
#include <memory>

namespace killme
{
    class AudioClip;

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

        VoiceUniquePtr<IXAudio2SourceVoice> sourceVoice_;
        std::shared_ptr<const AudioClip> clip_;
        VoiceCallback callBack_;
        bool isPlaying_;

    public:
        /** Constructs with an audio clip */
        SourceVoice(IXAudio2* xAudio, const std::shared_ptr<const AudioClip>& clip);

        /** Destructs */
        ~SourceVoice();

        /** Plays the audio at once */
        void play();

        /** Stops the audio */
        void stop();

        /** Pauses the audio */
        void pause();

        /** Returns true if the audio is playing now */
        bool isPlaying() const;
    };
}

#endif