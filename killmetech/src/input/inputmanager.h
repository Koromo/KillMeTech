#ifndef _KILLME_INPUTMANAGER_H_
#define _KILLME_INPUTMANAGER_H_

#include "keycode.h"
#include <array>
#include <vector>
#include <memory>

namespace killme
{
    class Event;
    class EventConnection;

    /** The input manager */
    class InputManager
    {
    private:
        std::array<bool, NUM_KEY_CODES> keyStatus_;
        std::vector<EventConnection> hooks_;

    public:
        /** Initializes */
        void startup();

        /** Finalizes */
        void shutdown();

    private:
        void onWinKeyDown(const Event& e);
        void onWinKeyUp(const Event& e);
    };

    extern InputManager inputManager;
}

#endif