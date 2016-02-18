#ifndef _KILLME_RUNTIME_H_
#define _KILLME_RUNTIME_H_

#include "../core/string.h"

namespace killme
{
    class LevelDesigner;

    /** KillMe Tech runtime engine */
    struct RunTime
    {
        /** Initializes KillMe Tech */
        static void startup(size_t width, size_t height, const tstring& title);

        /** Finalizes KillMe Tech */
        static void shutdown();

        /** Starts game */
        static void run(LevelDesigner& designer);
        static void run(LevelDesigner&& designer);

        /** Requests to exit game */
        static void quit();
    };
}

#endif