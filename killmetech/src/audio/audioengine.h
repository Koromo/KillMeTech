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

    /** Audio core class */
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
        /** Initialize audio engine */
        AudioEngine();

        /** Finalize audio engine */
        ~AudioEngine();

        /** Create a source voice */
        std::shared_ptr<SourceVoice> createSourceVoice(const Resource<AudioClip>& clip);

        /** Set the 3D listener parameters */
        void set3DListener(const ListenerParams& params);

        /** Calculate the 3D audio parameters and apply it */
        void apply3DEmission(const EmitterParams& params);
        
        /** Apply 3D audio parameters as the silent */
        void apply3DSilent(const std::shared_ptr<SourceVoice>& voice);
    };
}

#endif