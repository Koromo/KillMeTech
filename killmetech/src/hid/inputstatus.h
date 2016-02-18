#ifndef _KILLME_INPUTSTATUS_H_
#define _KILLME_INPUTSTATUS_H_

#include "keycode.h"
#include "../events/event.h"
#include <array>
#include <queue>
#include <Windows.h>

namespace killme
{
    /** Input event definitions */
    struct InputEvents
    {
        static const std::string keyPressed;
        static const std::string keyReleased;
    };

    /** Input status */
    class InputStatus
    {
    private:
        std::array<bool, NUM_KEY_CODES> keyStatus_;
        std::queue<Event> eventQueue_;

    public:
        /** Constructs */
        InputStatus();

        /** Whether any input events is queued or not */
        bool eventQueued() const;

        /** Pop the input event queue */
        Event popQueuedEvent();

        /** You need call on windows message resieved */
        void onKeyDown(WPARAM vkc);
        void onKeyUp(WPARAM vkc);
    };
}

#endif