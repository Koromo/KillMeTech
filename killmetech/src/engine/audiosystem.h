#ifndef _KILLME_AUDIOSYSTEM_H_
#define _KILLME_AUDIOSYSTEM_H_

#include "../resources/resource.h"
#include <memory>
#include <string>

namespace killme
{
    class AudioClip;
    class SourceVoice;
    class AudioManager;
    struct AudioDeviceDetails;

    /** Audio subsystem */
    class AudioSystem
    {
    private:
        std::unique_ptr<AudioManager> manager_;

    public:
        /** Startup audio subsystem */
        void startup();

        /** Shutdown audio subsysem */
        void shutdown();

        /** Create source voice */
        std::shared_ptr<SourceVoice> createSourceVoice(const Resource<AudioClip>& clip);

        /** Return the device details */
        AudioDeviceDetails getDeviceDetails() const;
    };

    extern AudioSystem audioSystem;
}

#endif