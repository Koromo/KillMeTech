#ifndef _KILLME_INPUTS_H_
#define _KILLME_INPUTS_H_

#include <Windows.h>

namespace killme
{
    struct Inputs
    {
        static void emitInputEvents();
        static void onWinKeyDown(WPARAM vkc);
        static void onWinKeyUp(WPARAM vkc);
    };
}

#endif