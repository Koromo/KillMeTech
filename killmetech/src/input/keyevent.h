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
        /** Event type definition */
        static const std::string eventType;

        /** Construct with event key */
        explicit KeyPressed(KeyCode key) : Event(KeyPressed::eventType), key_(key) {}

        /** Returns pressed key */
        KeyCode getKey() const { return key_; }
    };

    const std::string KeyPressed::eventType = "KeyPressed";

	/** Key Released event */
	class KeyReleased : public Event
	{
	private:
		KeyCode key_;

	public:
        /** Event type definition */
        static const std::string eventType;

		/** Construct with event key */
		explicit KeyReleased(KeyCode key) : Event(KeyReleased::eventType), key_(key) {}

		/** Returns pressed key */
        KeyCode getKey() const { return key_; }
	};

    const std::string KeyReleased::eventType = "KeyReleased";
}

#endif