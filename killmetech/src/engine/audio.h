#ifndef _KILLME_AUDIO_H_
#define _KILLME_AUDIO_H_

#include "../resources/resource.h"
#include <memory>
#include <string>

namespace killme
{
    class AudioClip;
    class SourceVoice;
    struct ListenerParams;
    struct EmitterParams;

    /** Audio subsystem interface */
    struct Audio
    {
        /** Startup audio subsystem */
        static void startup();

        /** Shutdown audio subsysem */
        static void shutdown();

        /** Create source voice */
        static std::shared_ptr<SourceVoice> createSourceVoice(const Resource<AudioClip>& clip);

        /** Set audio listener for calclation 3D audio */
        static void set3DListener(const ListenerParams& params);

        /** Set audio listener to null that means 3d audio is not emitted */
        static void setNull3DListener();

        /** Calclate 3d audio and apply into voice */
        static void apply3DEmission(const EmitterParams& params);
    };
}

#endif