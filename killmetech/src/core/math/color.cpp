#include "color.h"

namespace killme
{
    const Color Color::red = {1, 0, 0, 1};
    const Color Color::green = {0, 1, 0, 1};
    const Color Color::blue = {0, 0, 1, 1};
    const Color Color::white = {1, 1, 1, 1};
    const Color Color::black = {0, 0, 0, 1};

    Color::Color()
        : Color(Color::black)
    {
    }

    Color::Color(float r_, float g_, float b_, float a_)
        : r(r_)
        , g(g_)
        , b(b_)
        , a(a_)
    {
    }
}