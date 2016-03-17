#include "image.h"
#include "../core/math/math.h"
#include "../core/exception.h"
#include <string>
#include <cstdio>
#include <cstdint>

namespace killme
{
    namespace
    {
        const uint16_t FILE_TYPE = 0x4d42; // Now, support little endian only
        const uint32_t INFO_SIZE = 40; // Now, support windows header only 
        const uint16_t INFO_PLANES = 1;
        const uint16_t INFO_BIT_COUNT_24 = 24;
        const uint32_t INFO_COMPRESSION = 0; // Support Non compression only
        const uint32_t INFO_CLR_USED = 0;

#pragma pack(2)
        struct BmpFileHeader
        {
            uint16_t type;
            uint32_t size;
            uint16_t reserved1;
            uint16_t reserved2;
            uint32_t offbits;
        };
#pragma pack()

        struct BmpInfoHeader
        {
            uint32_t size;
            int32_t width;
            int32_t height;
            uint16_t planes;
            uint16_t bitCount;
            uint32_t compression;
            uint32_t sizeImage;
            int32_t pelsPerMeterX;
            int32_t pelsPerMeterY;
            uint32_t clrUsed;
            uint32_t clrImportant;
        };

        bool checkFormat(const BmpFileHeader& bf, const BmpInfoHeader& bi)
        {
            return
                bf.type == FILE_TYPE    &&
                bi.size == INFO_SIZE    &&
                bi.height > 0           &&
                bi.planes == INFO_PLANES            &&
                bi.bitCount == INFO_BIT_COUNT_24    &&
                bi.compression == INFO_COMPRESSION  &&
                bi.clrUsed == INFO_CLR_USED;
        }
    }

    std::shared_ptr<Image> decodeBmpImage(const std::string& path)
    {
        auto file = enforce<ImageLoadException>(std::fopen(path.c_str(), "rb"), "Failed to open file (" + path + ").");
        KILLME_SCOPE_EXIT{ fclose(file); };

        // Read file header and info header
        BmpFileHeader fileHeader;
        BmpInfoHeader infoHeader;
        enforce<ImageLoadException>(std::fread(&fileHeader, sizeof(fileHeader), 1, file) == 1, "Invalid .bmp file format.");
        enforce<ImageLoadException>(std::fread(&infoHeader, sizeof(infoHeader), 1, file) == 1, "Invalid .bmp file format.");
        enforce<ImageLoadException>(checkFormat(fileHeader, infoHeader), "Not supportted .bmp file format.");

        // Read color map
        auto image = std::make_shared<Image>(infoHeader.width, infoHeader.height);
        const auto stride = ceiling(infoHeader.width * (infoHeader.bitCount / 8), 4);
        std::vector<uint8_t> buffer(stride);

        for (int32_t y = infoHeader.height - 1; y >= 0; --y)
        {
            enforce<ImageLoadException>(std::fread(buffer.data(), stride, 1, file) == 1, "Invalid .bmp file format.");
            auto it = std::cbegin(buffer);
            for (int32_t x = 0; x < infoHeader.width; ++x)
            {
                image->at(x, y).b = *it; ++it;
                image->at(x, y).g = *it; ++it;
                image->at(x, y).r = *it; ++it;
                image->at(x, y).a = 1;
            }
        }

        return image;
    }
}