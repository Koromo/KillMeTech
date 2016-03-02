#include "audioclip.h"

namespace killme
{
    AudioClip::AudioClip(const unsigned char* data, size_t size, const WAVEFORMATEX& format)
        : data_(data)
        , size_(size)
        , format_(format)
    {
    }

    const unsigned char* AudioClip::getData() const
    {
        return data_.get();
    }

    size_t AudioClip::getSize() const
    {
        return size_;
    }

    WAVEFORMATEX AudioClip::getFormat() const
    {
        return format_;
    }

    AudioLoadException::AudioLoadException(const std::string& msg)
        : FileException(msg)
    {
    }

    std::shared_ptr<AudioClip> loadWavAudio(const tstring& path)
    {
        // Open file
        const auto mmio = enforce<AudioLoadException>(
            mmioOpen(const_cast<TCHAR*>(path.c_str()), nullptr, MMIO_READ),
            "Failed to open file (" + narrow(path) + ")."
            );

        KILLME_SCOPE_EXIT{ mmioClose(mmio, 0); };

        // Discend into RIFF chank
        MMCKINFO riffChunk;
        riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
        enforce<AudioLoadException>(
            mmioDescend(mmio, &riffChunk, nullptr, MMIO_FINDRIFF) == MMSYSERR_NOERROR,
            "Invalid .wav file format.");

        // Discend into FMT chank
        MMCKINFO fmtChunk;
        fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
        enforce<AudioLoadException>(
            mmioDescend(mmio, &fmtChunk, &riffChunk, MMIO_FINDCHUNK) == MMSYSERR_NOERROR,
            "Invalid .wav file format.");

        // Read audio format
        WAVEFORMATEX format;
        enforce<AudioLoadException>(
            mmioRead(mmio, reinterpret_cast<LPSTR>(&format), fmtChunk.cksize) == fmtChunk.cksize,
            "Invalid .wav file format.");

        // Ascend from FMT chack
        enforce<AudioLoadException>(
            mmioAscend(mmio, &fmtChunk, 0) == MMSYSERR_NOERROR,
            "Invalid .wav file format.");

        // Discend into DATA chank
        MMCKINFO dataChunk;
        dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
        enforce<AudioLoadException>(
            mmioDescend(mmio, &dataChunk, &riffChunk, MMIO_FINDCHUNK) == MMSYSERR_NOERROR,
            "Invalid .wav file format.");

        // Read audio data
        const auto data = new char[dataChunk.cksize];
        enforce<AudioLoadException>(
            mmioRead(mmio, data, dataChunk.cksize) == dataChunk.cksize,
            "Invalid .wav file format.");

        return std::make_shared<AudioClip>(reinterpret_cast<unsigned char*>(data), dataChunk.cksize, format);
    }
}