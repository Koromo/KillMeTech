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

        /** No argments constructor. Initial value is Color::black. */
        Color();

        /** Construct with initial value */
        Color(float r_, float g_, float b_, float a_);

        /** Specific color definitions */
        static const Color red;
        static const Color green;
        static const Color blue;
        static const Color white;
        static const Color black;
    };
}

#endif