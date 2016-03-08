#include "inputmanager.h"
#include "level.h"
#include <string>
#include <cassert>

namespace killme
{
    InputManager inputManager;

    namespace
    {
        std::string getKeyEventName(Keycode key)
        {
            return "Key_" + to<std::string>(key);
        }
    }

#define KILLME_DEFINE_KEY_EVENT(key) const std::string InputEvents::key = getKeyEventName(Keycode::key)
    KILLME_DEFINE_KEY_EVENT(Esc);
    KILLME_DEFINE_KEY_EVENT(A);
    KILLME_DEFINE_KEY_EVENT(B);
    KILLME_DEFINE_KEY_EVENT(C);
    KILLME_DEFINE_KEY_EVENT(D);
    KILLME_DEFINE_KEY_EVENT(E);
    KILLME_DEFINE_KEY_EVENT(F);
    KILLME_DEFINE_KEY_EVENT(G);
    KILLME_DEFINE_KEY_EVENT(H);
    KILLME_DEFINE_KEY_EVENT(I);
    KILLME_DEFINE_KEY_EVENT(J);
    KILLME_DEFINE_KEY_EVENT(K);
    KILLME_DEFINE_KEY_EVENT(L);
    KILLME_DEFINE_KEY_EVENT(M);
    KILLME_DEFINE_KEY_EVENT(N);
    KILLME_DEFINE_KEY_EVENT(O);
    KILLME_DEFINE_KEY_EVENT(P);
    KILLME_DEFINE_KEY_EVENT(Q);
    KILLME_DEFINE_KEY_EVENT(R);
    KILLME_DEFINE_KEY_EVENT(S);
    KILLME_DEFINE_KEY_EVENT(T);
    KILLME_DEFINE_KEY_EVENT(U);
    KILLME_DEFINE_KEY_EVENT(V);
    KILLME_DEFINE_KEY_EVENT(W);
    KILLME_DEFINE_KEY_EVENT(X);
    KILLME_DEFINE_KEY_EVENT(Y);
    KILLME_DEFINE_KEY_EVENT(Z);
#undef KILLME_DEFINE_KEY_EVENT

    InputManager::InputManager()
        : keyStatus_()
        , eventQueue_()
    {
        keyStatus_.fill(false);
    }

    namespace
    {
        // Convert WINAPI keycode to KillMeTech API key code
        Keycode toKeycode(WPARAM vkc)
        {
            switch (vkc)
            {
            case VK_ESCAPE: return Keycode::Esc;

            case 0x41: return Keycode::A;
            case 0x42: return Keycode::B;
            case 0x43: return Keycode::C;
            case 0x44: return Keycode::D;
            case 0x45: return Keycode::E;
            case 0x46: return Keycode::F;
            case 0x47: return Keycode::G;
            case 0x48: return Keycode::H;
            case 0x49: return Keycode::I;
            case 0x4a: return Keycode::J;
            case 0x4b: return Keycode::K;
            case 0x4c: return Keycode::L;
            case 0x4d: return Keycode::M;
            case 0x4e: return Keycode::N;
            case 0x4f: return Keycode::O;
            case 0x50: return Keycode::P;
            case 0x51: return Keycode::Q;
            case 0x52: return Keycode::R;
            case 0x53: return Keycode::S;
            case 0x54: return Keycode::T;
            case 0x55: return Keycode::U;
            case 0x56: return Keycode::V;
            case 0x57: return Keycode::W;
            case 0x58: return Keycode::X;
            case 0x59: return Keycode::Y;
            case 0x5a: return Keycode::Z;

            default:
                return Keycode::none;
            }
        }

        // Get index of key code
        size_t keyIndex(Keycode key)
        {
            return static_cast<size_t>(key);
        }
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
        if (key == Keycode::none)
        {
            return;
        }

        const auto i = keyIndex(key);
        if (!keyStatus_[i])
        {
            keyStatus_[i] = true;
            eventQueue_.emplace(getKeyEventName(key), true, false);
        }
    }

    void InputManager::onWinKeyUp(WPARAM vkc)
    {
        const auto key = toKeycode(vkc);
        if (key == Keycode::none)
        {
            return;
        }

        const auto i = keyIndex(key);
        if (keyStatus_[i])
        {
            keyStatus_[i] = false;
            eventQueue_.emplace(getKeyEventName(key), false, true);
        }
    }
}