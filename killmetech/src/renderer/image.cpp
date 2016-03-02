#include "image.h"
#include <cassert>

namespace killme
{
    Image::Image(size_t width, size_t height)
        : map_(width * height)
        , width_(width)
        , height_(height)
    {
    }

    size_t Image::getWidth() const
    {
        return width_;
    }

    size_t Image::getHeight() const
    {
        return height_;
    }

    const Pixel& Image::at(size_t x, size_t y) const
    {
        assert(x < width_ && y < height_ && "Index out of range.");
        return map_[width_ * y + x];
    }

    Pixel& Image::at(size_t x, size_t y)
    {
        return const_cast<Pixel&>(static_cast<const Image&>(*this).at(x, y));
    }

    const Pixel* Image::getPixelBuffer() const
    {
        return map_.data();
    }

    ImageLoadException::ImageLoadException(const std::string& msg)
        : FileException(msg)
    {
    }
}