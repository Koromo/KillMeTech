#include "inputs.h"
#include "events.h"
#include "../hid/inputstatus.h"

namespace killme
{
    namespace
    {
        InputStatus inputStatus_;
    }

    void Inputs::emitInputEvents()
    {
        while (inputStatus_.eventQueued())
        {
            Events::emit(inputStatus_.popQueuedEvent());
        }
    }

    void Inputs::onWinKeyDown(WPARAM vkc)
    {
        inputStatus_.onKeyDown(vkc);
    }

    void Inputs::onWinKeyUp(WPARAM vkc)
    {
        inputStatus_.onKeyUp(vkc);
    }
}