#include "image.h"
#include <cassert>

namespace killme
{
    Image::Image(size_t width, size_t height)
        : pixels_(width * height)
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

    const Pixel_r8g8b8a8& Image::at(size_t x, size_t y) const
    {
        assert(x < width_ && y < height_ && "Index out of range.");
        return pixels_[width_ * y + x];
    }

    Pixel_r8g8b8a8& Image::at(size_t x, size_t y)
    {
        return const_cast<Pixel_r8g8b8a8&>(static_cast<const Image&>(*this).at(x, y));
    }

    const Pixel_r8g8b8a8* Image::getPixels() const
    {
        return pixels_.data();
    }

    ImageLoadException::ImageLoadException(const std::string& msg)
        : FileException(msg)
    {
    }
}