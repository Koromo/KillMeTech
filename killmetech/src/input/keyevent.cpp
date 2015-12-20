#include "keyevent.h"

namespace killme
{
    const std::string KeyPressed::eventType = "KeyPressed";

    KeyPressed::KeyPressed(KeyCode key)
        : Event(KeyPressed::eventType)
        , key_(key)
    {
    }

    KeyCode KeyPressed::getKey() const
    {
        return key_;
    }

    const std::string KeyReleased::eventType = "KeyReleased";

	KeyReleased::KeyReleased(KeyCode key)
		: Event(KeyReleased::eventType)
		, key_(key)
	{
	}

	KeyCode KeyReleased::getKey() const
	{
		return key_;
	}
}