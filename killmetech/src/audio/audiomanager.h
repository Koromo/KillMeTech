#ifndef _KILLME_AUDIOMANAGER_H_
#define _KILLME_AUDIOMANAGER_H_

#include "xaudiosupport.h"
#include "../windows/winsupport.h"
#include "../resources/resource.h"
#include <xaudio2.h>
#include <memory>

namespace killme
{
    class AudioWorld;
    class AudioClip;
    class SourceVoice;

    /** Audio device properties */
    struct AudioDeviceDetails
    {
        size_t numSrcCannels;
        size_t numDestCannels;
        unsigned long channelMask;
    };

    /** Audio system */
    class AudioManager
    {
    private:
        ComSharedPtr<IXAudio2> xAudio_;
        VoiceUniquePtr<IXAudio2MasteringVoice> masteringVoice_;
        AudioDeviceDetails deviceDetails_;

    public:
        /** Initialize audio system */
        AudioManager();

        /** Finalize audio system */
        ~AudioManager();

        /** Return the audio device details */
        AudioDeviceDetails getDeviceDetails() const;

        /** Create a source voice */
        std::shared_ptr<SourceVoice> createSourceVoice(const Resource<AudioClip>& clip);
    };
}

#endif