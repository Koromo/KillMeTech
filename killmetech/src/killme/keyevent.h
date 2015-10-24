#ifndef _KILLME_KEYEVENT_H_
#define _KILLME_KEYEVENT_H_

#include "event.h"

namespace killme
{
    enum class KeyCode;

    /** Key event */
    class KeyEvent : public Event
    {
    private:
        KeyCode key_;

    public:
        /** Construct with event key */
        KeyEvent(KeyCode key, bool pressed);

        /** Returns pressed key */
        KeyCode getKey() const;
    };
}

#endif