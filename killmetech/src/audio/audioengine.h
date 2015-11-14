#ifndef _KILLME_AUDIOENGINE_H_
#define _KILLME_AUDIOENGINE_H_

#include "../windows/winsupport.h"
#include "xaudiosupport.h"
#include <xaudio2.h>
#include <memory>

namespace killme
{
    class AudioClip;
    class SourceVoice;

    /** Audio core class */
    class AudioEngine
    {
    private:
        ComSharedPtr<IXAudio2> xAudio_;
        VoiceUniquePtr<IXAudio2MasteringVoice> masteringVoice_;

    public:
        /** Startup audio engine */
        AudioEngine();

        /** Shutdown audio engine */
        ~AudioEngine();

        /** Create source voice */
        /// TODO: Memoty error will occur if we destroy AudioEngine before destroy SourceVoice
        std::shared_ptr<SourceVoice> createSourceVoice(const std::shared_ptr<const AudioClip>& clip);
    };
}

#endif