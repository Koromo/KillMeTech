#include "keyevent.h"

namespace killme
{
    KeyEvent::KeyEvent(KeyCode key, bool pressed)
        : Event(pressed ? "KeyPressed" : "KeyReleased")
        , key_(key)
    {
    }

    KeyCode KeyEvent::getKey() const
    {
        return key_;
    }
}