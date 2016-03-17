#ifndef _KILLME_IMAGE_H_
#define _KILLME_IMAGE_H_

#include "pixels.h"
#include "../core/exception.h"
#include <vector>
#include <memory>
#include <string>

namespace killme
{
    /** Image buffer */
    class Image
    {
    private:
        std::vector<Pixel_r8g8b8a8> pixels_;
        size_t width_;
        size_t height_;

    public:
        /** Construct with a buffer size */
        Image(size_t width, size_t height);

        /** Return dimention size */
        size_t getWidth() const;
        size_t getHeight() const;

        /** Return pixel */
        const Pixel_r8g8b8a8& at(size_t x, size_t y) const;
        Pixel_r8g8b8a8& at(size_t x, size_t y);

        /** Return pixel buffer */
        const Pixel_r8g8b8a8* getPixels() const;
    };

    /** Loading image exception */
    class ImageLoadException : public FileException
    {
    public:
        /** Construct */
        explicit ImageLoadException(const std::string& msg);
    };

    /** Codec functions */
    std::shared_ptr<Image> decodeBmpImage(const std::string& path);
}

#endif