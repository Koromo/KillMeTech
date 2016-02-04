#ifndef _KILLME_AUDIOMANAGER_H_
#define _KILLME_AUDIOMANAGER_H_

#include "../windows/winsupport.h"
#include "xaudiosupport.h"
#include <xaudio2.h>
#include <memory>

namespace killme
{
    class AudioClip;
    class SourceVoice;

    /** Audio core class */
    class AudioManager
    {
    private:
        ComSharedPtr<IXAudio2> xAudio_;
        VoiceUniquePtr<IXAudio2MasteringVoice> masteringVoice_;

    public:
        /** Startups audio manager */
        AudioManager();

        /** Shutdowns audio manager */
        ~AudioManager();

        /** Creates source voice */
        std::shared_ptr<SourceVoice> createSourceVoice(const std::shared_ptr<const AudioClip>& clip);
    };
}

#endif