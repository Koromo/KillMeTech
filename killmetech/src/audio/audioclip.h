#ifndef _KILLME_AUDIOCLIP_H_
#define _KILLME_AUDIOCLIP_H_

#include "../resources/resource.h"
#include "../core/string.h"
#include <Windows.h>
#include <memory>

namespace killme
{
    /** The Audio clip */
    class AudioClip : public IsResource
    {
    private:
        std::unique_ptr<const unsigned char[]> data_;
        size_t size_;
        WAVEFORMATEX format_;

    public:
        /** Constructs with an audio data */
        AudioClip(const unsigned char* data, size_t size, const WAVEFORMATEX& format);

        /** Returns the audio data */
        const unsigned char* getData() const;

        /** Returns the size[byte] of the audio data */
        size_t getSize() const;

        /** Returns the audio format */
        WAVEFORMATEX getFormat() const;
    };

    /** Load an audio from file */
    std::shared_ptr<AudioClip> loadAudioClip(const tstring& path);
}

#endif