#ifndef _KILLME_AUDIOENGINE_H_
#define _KILLME_AUDIOENGINE_H_

#include "xaudiosupport.h"
#include "../windows/winsupport.h"
#include "../resources/resource.h"
#include <xaudio2.h>
#include <Windows.h>

#define _XM_NO_INTRINSICS_ 1;
#include <x3daudio.h>

#include <memory>
#include <vector>

namespace killme
{
    class AudioClip;
    class SourceVoice;
    struct ListenerParams;
    struct EmitterParams;

    /** The Audio core class */
    class AudioEngine
    {
    private:
        ComSharedPtr<IXAudio2> xAudio_;
        VoiceUniquePtr<IXAudio2MasteringVoice> masteringVoice_;

        X3DAUDIO_HANDLE x3DAudio_;
        X3DAUDIO_LISTENER listener_;
        size_t numSrcCannels_;
        size_t numDestCannels_;
        std::vector<float> levelMatrix_;

    public:
        /** Initializes audio engine */
        AudioEngine();

        /** Finalizes audio engine */
        ~AudioEngine();

        /** Creates a source voice */
        std::shared_ptr<SourceVoice> createSourceVoice(const Resource<AudioClip>& clip);

        /** Sets the 3D listener parameters */
        void set3DListener(const ListenerParams& params);

        /** Calculates 3D audio parameters and apply it */
        void apply3DEmission(const EmitterParams& params);
        void apply3DSilent(const std::shared_ptr<SourceVoice>& voice);
    };
}

#endif