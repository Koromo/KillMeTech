#ifndef _KILLME_IMAGE_H_
#define _KILLME_IMAGE_H_

#include <vector>
#include <memory>
#include <cstdint>

namespace killme
{
    struct Pixel
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    /** Image buffer */
    class Image
    {
    private:
        std::vector<Pixel> map_;
        size_t width_;
        size_t height_;

    public:
        /** Construct with a size */
        Image(size_t width, size_t height);

        /** Return size */
        size_t getWidth() const;
        size_t getHeight() const;

        /** Return pixel */
        const Pixel& at(size_t x, size_t y) const;
        Pixel& at(size_t x, size_t y);
        const Pixel* getPixelBuffer() const;
    };

    /** Codec functions */
    std::shared_ptr<Image> decodeBmp(const std::string& path);
}

#endif