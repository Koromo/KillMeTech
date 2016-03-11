#include "inputmanager.h"
#include "level.h"
#include "eventdef.h"
#include <string>
#include <tuple>
#include <cassert>

namespace killme
{
    InputManager inputManager;

    InputManager::InputManager()
        : keyStatus_()
        , eventQueue_()
    {
        keyStatus_.fill(false);
    }

    namespace
    {
#define KILLME_CASE_VKEY(vkc, key) case vkc: return std::make_tuple(Keycode::key, static_cast<size_t>(Keycode::key), LEVEL_##key)
        // Convert WINAPI keycode to KillMeTech API key code
        std::tuple<Keycode, size_t, std::string> toKeycode(WPARAM vkc)
        {
            switch (vkc)
            {
                KILLME_CASE_VKEY(VK_ESCAPE, Esc);
                KILLME_CASE_VKEY(0x41, A);
                KILLME_CASE_VKEY(0x42, B);
                KILLME_CASE_VKEY(0x43, C);
                KILLME_CASE_VKEY(0x44, D);
                KILLME_CASE_VKEY(0x45, E);
                KILLME_CASE_VKEY(0x46, F);
                KILLME_CASE_VKEY(0x47, G);
                KILLME_CASE_VKEY(0x48, H);
                KILLME_CASE_VKEY(0x49, I);
                KILLME_CASE_VKEY(0x4a, J);
                KILLME_CASE_VKEY(0x4b, K);
                KILLME_CASE_VKEY(0x4c, L);
                KILLME_CASE_VKEY(0x4d, M);
                KILLME_CASE_VKEY(0x4e, N);
                KILLME_CASE_VKEY(0x4f, O);
                KILLME_CASE_VKEY(0x50, P);
                KILLME_CASE_VKEY(0x51, Q);
                KILLME_CASE_VKEY(0x52, R);
                KILLME_CASE_VKEY(0x53, S);
                KILLME_CASE_VKEY(0x54, T);
                KILLME_CASE_VKEY(0x55, U);
                KILLME_CASE_VKEY(0x56, V);
                KILLME_CASE_VKEY(0x57, W);
                KILLME_CASE_VKEY(0x58, X);
                KILLME_CASE_VKEY(0x59, Y);
                KILLME_CASE_VKEY(0x5a, Z);

            default:
                return std::make_tuple(Keycode::none, 0, LEVEL_A);
            }
        }
#undef KILLME_CASE_KEY
    }

    void InputManager::emitInputEvents(Level& level)
    {
        while (!eventQueue_.empty())
        {
            level.emit(eventQueue_.front());
            eventQueue_.pop();
        }
    }

    void InputManager::onWinKeyDown(WPARAM vkc)
    {
        const auto key = toKeycode(vkc);
        if (std::get<0>(key) == Keycode::none)
        {
            return;
        }

        const auto i = std::get<1>(key);
        if (!keyStatus_[i])
        {
            keyStatus_[i] = true;
            eventQueue_.emplace(std::get<2>(key), true, false);
        }
    }

    void InputManager::onWinKeyUp(WPARAM vkc)
    {
        const auto key = toKeycode(vkc);
        if (std::get<0>(key) == Keycode::none)
        {
            return;
        }

        const auto i = std::get<1>(key);
        if (keyStatus_[i])
        {
            keyStatus_[i] = false;
            eventQueue_.emplace(std::get<2>(key), false, true);
        }
    }
}