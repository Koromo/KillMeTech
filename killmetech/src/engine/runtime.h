#ifndef _KILLME_RUNTIME_H_
#define _KILLME_RUNTIME_H_

#include "../core/string.h"

namespace killme
{
    class LevelDesigner;

    /** Frame rate definition */
    enum class FrameRate : size_t
    {
        noLimit,
        _120,
        _60,
        _30,
        _20,
        _15
    };

    /** KillMe Tech runtime engine */
    struct RunTime
    {
        /** Initializes KillMe Tech */
        static void startup(size_t width, size_t height, const tstring& title);

        /** Finalizes KillMe Tech */
        static void shutdown();

        /** Set FPS */
        static void setFrameRate(FrameRate fps);

        /** Return delta time[s] of current frame from previous */
        static float getDeltaTime();

        /** Return average fps of current n frame */
        /// NOTE: n <= 120
        static float getCurrentFrameRate(size_t n);

        /** Starts game */
        static void run(LevelDesigner& designer);
        static void run(LevelDesigner&& designer);

        /** Requests to exit game */
        static void quit();
    };
}

#endif