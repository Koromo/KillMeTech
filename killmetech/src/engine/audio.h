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

    struct Audio
    {
        static void startup();
        static void shutdown();
        static std::shared_ptr<SourceVoice> createSourceVoice(const Resource<AudioClip>& clip);
        static void set3DListener(const ListenerParams& params);
        static void apply3DEmission(const EmitterParams& params);
        static void setNull3DListener();
    };
}

#endif