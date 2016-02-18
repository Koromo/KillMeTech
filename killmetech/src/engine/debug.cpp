#include "debug.h"
#include "graphics.h"
#include "../scene/debugdrawmanager.h"
#include "../renderer/rendersystem.h"
#include "../windows/console.h"

namespace killme
{
    Console& Debug::console = killme::console;

    void Debug::startup()
    {
        debugDrawManager.startup(Graphics::getRenderSystem());
        Debug::console.allocate();
    }

    void Debug::shutdown()
    {
        Debug::console.free();
        debugDrawManager.shutdown();
    }

    void Debug::line(const Vector3& from, const Vector3& to, const Color& color)
    {
        debugDrawManager.line(from, to, color);
    }

    void Debug::debugDraw()
    {
        const auto camera = Graphics::getMainCamera();
        if (camera)
        {
            debugDrawManager.debugDraw(*camera, Graphics::getRenderSystem()->getCurrentFrameResource());
        }
        else
        {
            debugDrawManager.clear();
        }
    }
}