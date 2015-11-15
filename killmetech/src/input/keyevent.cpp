#include "keyevent.h"

namespace killme
{
    KeyPressed::KeyPressed(KeyCode key)
        : Event("KeyPressed")
        , key_(key)
    {
    }

    KeyCode KeyPressed::getKey() const
    {
        return key_;
    }

	KeyReleased::KeyReleased(KeyCode key)
		: Event("KeyReleased")
		, key_(key)
	{
	}

	KeyCode KeyReleased::getKey() const
	{
		return key_;
	}
}