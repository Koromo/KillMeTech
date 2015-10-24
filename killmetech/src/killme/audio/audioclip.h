#ifndef _KILLME_AUDIOCLIP_H_
#define _KILLME_AUDIOCLIP_H_

#include "../core/string.h"
#include <Windows.h>
#include <memory>

namespace killme
{
    /** Audio clip abstruction */
    class AudioClip
    {
    private:
        std::unique_ptr<const unsigned char[]> data_;
        size_t size_;
        WAVEFORMATEX format_;

    public:
        /** Construct with audio data */
        AudioClip(const unsigned char* data, size_t size, const WAVEFORMATEX& format);

        /** Returns audio data */
        const unsigned char* getData() const;

        /** Returns size[byte] of audio data */
        size_t getSize() const;

        /** Returns audio format */
        WAVEFORMATEX getFormat() const;
    };

    /** Load audio from file */
    std::shared_ptr<AudioClip> loadAudio(const tstring& filename);
}

#endif