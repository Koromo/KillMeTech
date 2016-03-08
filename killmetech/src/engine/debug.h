#ifndef _KILLME_DEBUG_H_
#define _KILLME_DEBUG_H_

#include "../physics/physicsworld.h"
#include "../windows/console.h"
#include "../core/platform.h"
#include "../core/string.h"

#ifdef KILLME_DEBUG
namespace killme
{

    class Vector3;
    class Color;
    class Console;
    class Scene;
    struct FrameResource;

    namespace detail
    {
        struct Debug
        {
            class PhysicsDebugger : public PhysicsDebugDrawer
            {
                void line(const Vector3& from, const Vector3& to, const Color& color);
            };

            static void startup();
            static void shutdown();
            static void line(const Vector3& from, const Vector3& to, const Color& color);
            static void draw(Scene& world, const FrameResource& frame);
        };
    }
}

/** Initialize debugger */
#define KILLME_DEBUG_INITIALIZE() \
    (killme::detail::Debug::startup())

/** Finalize debugger */
#define KILLME_DEBUG_FINALIZE() \
    (killme::detail::Debug::shutdown())

/** Draw debug physics */
#define KILLME_DEBUG_DRAW_PHYSICS(physics) \
    (physics.debugDraw(std::make_shared<killme::detail::Debug::PhysicsDebugger>()))

/** Draw a line */
#define KILLME_DEBUG_LINE(from, to, color) \
    (killme::detail::Debug::line(from, to, color))

/** Draw debugs */
#define KILLME_DEBUG_DRAW(world, frame) \
    (killme::detail::Debug::draw(world, frame))

/** Allocate console */
#define KILLME_CONSOLE_ALLOC() \
    (killme::console.allocate())

/** Free console */
#define KILLME_CONSOLE_FREE() \
    (killme::console.free())

/** Read from console */
#define KILLME_CONSOLE_READ() \
    (killme::console.read())

/** Output to console */
#define KILLME_CONSOLE_WRITE(str) \
    (killme::console.write(str))

/** ditto */
#define KILLME_CONSOLE_WRITEF(fmt, ...) \
    (killme::console.writef(fmt, __VA_ARGS__))

/** ditto */
#define KILLME_CONSOLE_WRITELN(str) \
    (killme::console.writeln(str))

/** ditto */
#define KILLME_CONSOLE_WRITEFLN(fmt, ...) \
    (killme::console.writefln(fmt, __VA_ARGS__))

#elif

#define KILLME_DEBUG_INITIALIZE()
#define KILLME_DEBUG_FINALIZE()
#define KILLME_DEBUG_PHYSICS_WORLD(world)
#define KILLME_DEBUG_LINE(from, to, color)
#define KILLME_DEBUG_DRAW_DEBUGS(world, frame)
#define KILLME_CONSOLE_ALLOC()
#define KILLME_CONSOLE_FREE()
#define KILLME_CONSOLE_READ (KILLME_T(""))
#define KILLME_CONSOLE_WRITE(str)
#define KILLME_CONSOLE_WRITEF(fmt, ...)
#define KILLME_CONSOLE_WRITELN(str)
#define KILLME_CONSOLE_WRITEFLN(fmt, ...)

#endif

#endif