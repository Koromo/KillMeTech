#ifndef _KILLME_COLOR_H_
#define _KILLME_COLOR_H_

#include <initializer_list>

namespace killme
{
    /** RGBA color */
    class Color
    {
    public:
        float r, g, b, a;

        /** No argments constructor. Initial value is Color::BLACK. */
        Color();

        /** Construct with initial value */
        Color(float r_, float g_, float b_, float a_);

        /** Accessor */
        const float& operator[](size_t i) const;
        float& operator[](size_t i);

        /** Specific color definitions */
        static const Color RED;
        static const Color GREEN;
        static const Color BLUE;
        static const Color WHITE;
        static const Color BLACK;
    };
}

#endif