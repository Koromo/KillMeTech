#include "audioclip.h"
#include "string.h"
#include "exception.h"
#include "fileexception.h"
#include <memory>
#include <cassert>

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

    namespace
    {
        // For hold MMIO handle
        struct MMIODeleter
        {
            void operator()(HMMIO h)
            {
                mmioClose(h, 0);
            }
        };

        using MmioUniquePtr = std::unique_ptr<std::remove_pointer_t<HMMIO>, MMIODeleter>;
    }

    std::shared_ptr<AudioClip> loadAudio(const tstring& filename)
    {
        // Open file
        const auto mmio = enforce<FileException>(
            mmioOpen(const_cast<TCHAR*>(filename.c_str()), nullptr, MMIO_READ),
            "Failed to open file (" + narrow(filename) + ")."
            );

        // Hold mmio handle for safety
        const MmioUniquePtr mmioHolder(mmio, MMIODeleter());

        // Discend into RIFF chank
        MMCKINFO riffChunk;
        riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
        auto mr = mmioDescend(mmio, &riffChunk, nullptr, MMIO_FINDRIFF);
        assert(mr == MMSYSERR_NOERROR && "Failed to descend into RIFF chank.");

        // Discend into FMT chank
        MMCKINFO fmtChunk;
        fmtChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
        mr = mmioDescend(mmio, &fmtChunk, &riffChunk, MMIO_FINDCHUNK);
        assert(mr == MMSYSERR_NOERROR && "Failed to descend into FMT chank.");

        // Read audio format
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(format));
        auto readSize = mmioRead(mmio, reinterpret_cast<LPSTR>(&format), fmtChunk.cksize);
        assert(readSize == fmtChunk.cksize && "Failed to read audio format");

        // Ascend from FMT chack
        mr = mmioAscend(mmio, &fmtChunk, 0);
        assert(mr == MMSYSERR_NOERROR && "Failed to ascend from fmt chank");

        // Discend into DATA chank
        MMCKINFO dataChunk;
        dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
        mr = mmioDescend(mmio, &dataChunk, &riffChunk, MMIO_FINDCHUNK);
        assert(mr == MMSYSERR_NOERROR && "Failed to descend into data chank");

        // Read audio data
        const auto data = new char[dataChunk.cksize];
        readSize = mmioRead(mmio, data, dataChunk.cksize);
        assert(readSize == dataChunk.cksize && "Failed to read audio data");

        return std::make_shared<AudioClip>(reinterpret_cast<unsigned char*>(data), dataChunk.cksize, format);
    }
}