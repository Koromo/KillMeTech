#ifndef _KILLME_DEBUG_H_
#define _KILLME_DEBUG_H_

namespace killme
{
    struct Debug
    {
        static void startup();
        static void shutdown();
        static void debugDraw();
    };
}

#endif