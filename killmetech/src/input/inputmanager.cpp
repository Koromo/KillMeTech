#include "inputmanager.h"
#include "../event/event.h"
#include "../event/eventmanager.h"
#include <Windows.h>
#include <functional>

namespace killme
{
    InputManager inputManager;

    void InputManager::startup()
    {
        keyStatus_.fill(false);

        using namespace std::placeholders;
        hooks_.push_back(eventManager.connect(
            EventType::win_KeyDown, std::bind(&InputManager::onWinKeyDown, this, _1)));
        hooks_.push_back(eventManager.connect(
            EventType::win_KeyUp, std::bind(&InputManager::onWinKeyUp, this, _1)));
    }

    void InputManager::shutdown()
    {
        hooks_.clear();
    }

    namespace
    {
        // Convert WINAPI key code to KillMeTech API key code
        KeyCode toKeyCode(WPARAM vkc)
        {
            switch (vkc)
            {
            case VK_ESCAPE: return KeyCode::Esc;

            case 0x41: return KeyCode::A;
            case 0x42: return KeyCode::B;
            case 0x43: return KeyCode::C;
            case 0x44: return KeyCode::D;
            case 0x45: return KeyCode::E;
            case 0x46: return KeyCode::F;
            case 0x47: return KeyCode::G;
            case 0x48: return KeyCode::H;
            case 0x49: return KeyCode::I;
            case 0x4a: return KeyCode::J;
            case 0x4b: return KeyCode::K;
            case 0x4c: return KeyCode::L;
            case 0x4d: return KeyCode::M;
            case 0x4e: return KeyCode::N;
            case 0x4f: return KeyCode::O;
            case 0x50: return KeyCode::P;
            case 0x51: return KeyCode::Q;
            case 0x52: return KeyCode::R;
            case 0x53: return KeyCode::S;
            case 0x54: return KeyCode::T;
            case 0x55: return KeyCode::U;
            case 0x56: return KeyCode::V;
            case 0x57: return KeyCode::W;
            case 0x58: return KeyCode::X;
            case 0x59: return KeyCode::Y;
            case 0x5a: return KeyCode::Z;

            default:
                return KeyCode::none;
            }
        }

        // Get index of key code
        size_t keyIndex(KeyCode key)
        {
            return static_cast<size_t>(key);
        }
    }

    void InputManager::onWinKeyDown(const Event& e)
    {
        const WPARAM wp = e[0];
        const auto key = toKeyCode(wp);
        if (key == KeyCode::none)
        {
            return;
        }

        const auto i = keyIndex(key);
        if (!keyStatus_[i])
        {
            keyStatus_[i] = true;
            Event keyPressed(EventType::keyPressed, 1);
            keyPressed[0] = key;
            eventManager.emit(keyPressed);
        }
    }

    void InputManager::onWinKeyUp(const Event& e)
    {
        const WPARAM wp = e[0];
        const auto key = toKeyCode(wp);
        if (key == KeyCode::none)
        {
            return;
        }

        const auto i = keyIndex(key);
        if (keyStatus_[i])
        {
            keyStatus_[i] = false;
            Event keyReleased(EventType::keyReleased, 1);
            keyReleased[0] = key;
            eventManager.emit(keyReleased);
        }
    }
}