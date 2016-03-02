#ifndef _KILLME_COLOR_H_
#define _KILLME_COLOR_H_

namespace killme
{
    /** RGBA color */
    class Color
    {
    public:
        /** Elements */
        float r, g, b, a;

        /** Construct as the Color::BLACK */
        Color();

        /** Construct with initial values */
        Color(float r_, float g_, float b_, float a_);

        /** Construct */
        Color(const Color&) = default;
        Color(Color&&) = default;

        /** Assignment operator */
        Color& operator =(const Color&) = default;
        Color& operator =(Color&&) = default;

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