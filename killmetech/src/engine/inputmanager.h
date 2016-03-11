#ifndef _KILLME_INPUTMANAGER_H_
#define _KILLME_INPUTMANAGER_H_

#include "keycode.h"
#include "../events/event.h"
#include <Windows.h>
#include <array>
#include <queue>

namespace killme
{
    class Level;

    /** Input manager */
    class InputManager
    {
    private:
        std::array<bool, NUM_KEY_CODES> keyStatus_;
        std::queue<Event> eventQueue_;

    public:
        /** Construct */
        InputManager();

        /** Emit input events to a level */
        void emitInputEvents(Level& level);

        /** You need call on windows message resieved */
        void onWinKeyDown(WPARAM vkc);
        void onWinKeyUp(WPARAM vkc);
    };

    extern InputManager inputManager;
}

#endif