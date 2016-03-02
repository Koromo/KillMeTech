#ifndef _KILLME_AUDIOCLIP_H_
#define _KILLME_AUDIOCLIP_H_

#include "../resources/resource.h"
#include "../core/string.h"
#include "../core/exception.h"
#include <Windows.h>
#include <string>
#include <memory>

namespace killme
{
    /** Audio clip */
    class AudioClip : public IsResource
    {
    private:
        std::unique_ptr<const unsigned char[]> data_;
        size_t size_;
        WAVEFORMATEX format_;

    public:
        /** Construct */
        AudioClip(const unsigned char* data, size_t size, const WAVEFORMATEX& format);

        /** Return the audio data */
        const unsigned char* getData() const;

        /** Return the size[byte] of the audio data */
        size_t getSize() const;

        /** Return the audio format */
        WAVEFORMATEX getFormat() const;
    };

    /** Loading audio exception */
    class AudioLoadException : public FileException
    {
    public:
        /** Construct */
        explicit AudioLoadException(const std::string& msg);
    };

    /** Load an audio from .wav file */
    std::shared_ptr<AudioClip> loadWavAudio(const tstring& path);
}

#endif