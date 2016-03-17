#ifndef _KILLME_PIXELS_H_
#define _KILLME_PIXELS_H_

#include <cstdint> /// TODO: Environment dependence

namespace killme
{
    /** Pixel format definitions */
    enum class PixelFormat
    {
        r8g8b8a8_unorm
    };

    /** Pixel definition (Red:8 Green:8 Blue:8 Aplha:8) */
    struct Pixel_r8g8b8a8
    {
        uint8_t r, g, b, a;
        static const PixelFormat UNORM_FORMAT = PixelFormat::r8g8b8a8_unorm;
    };

    /** Return the count of bits of pixel format */
    size_t numBitsOfPixelFormat(PixelFormat pf);
}

#endif