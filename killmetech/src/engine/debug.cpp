#include "debug.h"
#include "graphicssystem.h"
#include "../scene/scene.h"
#include "../scene/debugdrawmanager.h"

namespace killme
{
#ifdef KILLME_DEBUG
    void detail::Debug::PhysicsDebugger::line(const Vector3& from, const Vector3& to, const Color& color)
    {
        debugDrawManager.line(from, to, color);
    }

    void detail::Debug::startup()
    {
        debugDrawManager.initialize(graphicsSystem.getRenderSystem());
    }

    void detail::Debug::shutdown()
    {
        debugDrawManager.finalize();
    }

    void detail::Debug::line(const Vector3& from, const Vector3& to, const Color& color)
    {
        debugDrawManager.line(from, to, color);
    }

    void detail::Debug::draw(Scene& world, const FrameResource& frame)
    {
        const auto camera = world.getMainCamera();
        if (camera)
        {
            debugDrawManager.debugDraw(*camera, frame);
        }
        else
        {
            debugDrawManager.clear();
        }
    }
#endif
}