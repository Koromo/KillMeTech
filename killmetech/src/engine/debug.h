#ifndef _KILLME_DEBUG_H_
#define _KILLME_DEBUG_H_

namespace killme
{
    class Vector3;
    class Color;
    class Console;

    /** Debug interface */
    struct Debug
    {
        /** Console interface */
        static Console& console;

        /** Startup */
        static void startup();

        /** Shutdown */
        static void shutdown();

        /** Draw line */
        static void line(const Vector3& from, const Vector3& to, const Color& color);

        /** Draw debugs */
        static void debugDraw();
    };
}

#endif