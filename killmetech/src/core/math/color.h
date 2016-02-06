#ifndef _KILLME_COLOR_H_
#define _KILLME_COLOR_H_

namespace killme
{
    /** The RGBA color */
    class Color
    {
    public:
        /** The elements */
        float r, g, b, a;

        /** Constructs the Color::BLACK */
        Color();

        /** Constructs with initial values */
        Color(float r_, float g_, float b_, float a_);

        /** Accessor */
        const float& operator [](size_t i) const;
        float& operator [](size_t i);

        /** Specific color definitions */
        static const Color RED;
        static const Color GREEN;
        static const Color BLUE;
        static const Color WHITE;
        static const Color BLACK;
    };
}

#endif