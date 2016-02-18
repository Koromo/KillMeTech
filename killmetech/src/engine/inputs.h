#ifndef _KILLME_INPUTS_H_
#define _KILLME_INPUTS_H_

#include <Windows.h>

namespace killme
{
    /** Human input device subsystem */
    struct Inputs
    {
        /** Triggr all input events */
        static void emitInputEvents();

        /** You need call on WM_KEYDOWN and UP */
        static void onWinKeyDown(WPARAM vkc);
        static void onWinKeyUp(WPARAM vkc);
    };
}

#endif