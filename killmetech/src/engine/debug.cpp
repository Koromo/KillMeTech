#include "debug.h"
#include "graphics.h"
#include "../scene/debugdrawmanager.h"
#include "../renderer/rendersystem.h"

namespace killme
{
    void Debug::startup()
    {
        debugDrawManager.startup(Graphics::getRenderSystem());
    }

    void Debug::shutdown()
    {
        debugDrawManager.shutdown();
    }

    void Debug::debugDraw()
    {
        const auto camera = Graphics::getMainCamera();
        if (camera)
        {
            debugDrawManager.debugDraw(*camera, Graphics::getRenderSystem()->getCurrentFrameResource());
        }
    }
}