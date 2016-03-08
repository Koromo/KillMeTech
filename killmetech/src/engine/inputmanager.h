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

    /** Input event definitions */
    /** NOTE: Input events parameters
     *        0: bool -> Whether key pressed or not
     *        1: bool -> Whether key released or not
     */
    struct InputEvents
    {
        static const std::string Esc;
        static const std::string A;
        static const std::string B;
        static const std::string C;
        static const std::string D;
        static const std::string E;
        static const std::string F;
        static const std::string G;
        static const std::string H;
        static const std::string I;
        static const std::string J;
        static const std::string K;
        static const std::string L;
        static const std::string M;
        static const std::string N;
        static const std::string O;
        static const std::string P;
        static const std::string Q;
        static const std::string R;
        static const std::string S;
        static const std::string T;
        static const std::string U;
        static const std::string V;
        static const std::string W;
        static const std::string X;
        static const std::string Y;
        static const std::string Z;
    };

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