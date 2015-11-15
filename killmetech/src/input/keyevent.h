#ifndef _KILLME_KEYEVENT_H_
#define _KILLME_KEYEVENT_H_

#include "../event/event.h"

namespace killme
{
    enum class KeyCode;

    /** Key pressed event */
    class KeyPressed : public Event
    {
    private:
        KeyCode key_;

    public:
        /** Construct with event key */
        explicit KeyPressed(KeyCode key);

        /** Returns pressed key */
        KeyCode getKey() const;
    };

	/** Key Released event */
	class KeyReleased : public Event
	{
	private:
		KeyCode key_;

	public:
		/** Construct with event key */
		explicit KeyReleased(KeyCode key);

		/** Returns pressed key */
		KeyCode getKey() const;
	};
}

#endif