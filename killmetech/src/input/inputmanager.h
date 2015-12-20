#ifndef _KILLME_INPUTMANAGER_H_
#define _KILLME_INPUTMANAGER_H_

#include "keycode.h"
#include <array>
#include <Windows.h>

namespace killme
{
    class EventDispatcher;

    // Input module manager
    class InputManager
    {
    private:
        std::array<bool, NUM_KEY_CODES> keyStatus_;

    public:
        /** Constructor */
        InputManager();

        /** Called on Win key event and dispatch key events */
        void onWinKeyEvent(EventDispatcher& dispatcher, UINT msg, WPARAM wp);
    };
}

#endif