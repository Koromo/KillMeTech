#ifndef _KILLME_XAUDIOSUPPORT_H_
#define _KILLME_XAUDIOSUPPORT_H_

#include "../core/exception.h"
#include <xaudio2.h>
#include <memory>
#include <string>

namespace killme
{
    /** XAudio relational exception */
    class XAudioException : public Exception
    {
    public:
        /** Construct with a message */
        explicit XAudioException(const std::string& msg)
            : Exception(msg) {}
    };

    namespace detail
    {
        // Deleter for XAudio voice objects
        template <class T>
        struct VoiceDeleter
        {
            void operator()(T* p)
            {
                if (p)
                {   
                    p->DestroyVoice();
                }
            }
        };
    }

    /** Smart pointers for XAudio voice objects */
    template <class T>
    using VoiceUniquePtr = std::unique_ptr<T, detail::VoiceDeleter<T>>;

    template <class T>
    using VoiceSharedPtr = std::shared_ptr<T>;

    /** Make helpers */
    template <class T>
    VoiceUniquePtr<T> makeVoiceUnique(T* p)
    {
        return VoiceUniquePtr<T>(p, detail::VoiceDeleter<T>());
    }

    template <class T>
    VoiceSharedPtr<T> makeVoiceShared(T* p)
    {
        return VoiceSharedPtr<T>(p, detail::VoiceDeleter<T>());
    }
}

#endif