#ifndef _KILLME_GRAPHICS_H_
#define _KILLME_GRAPHICS_H_

#include <Windows.h>
#include <memory>

namespace killme
{
    class Scene;
    class SceneNode;
    class Camera;
    class RenderSystem;
    struct Viewport;

    /** Rendering subsystem */
    struct Graphics
    {
        /** Startup rendering subsystem */
        static void startup(HWND window);

        /** Shutdown rendering subsystem */
        static void shutdown();

        /** Returns render system */
        static std::shared_ptr<RenderSystem> getRenderSystem();

        /** Returns viewport based on client size of render window */
        static Viewport getClientViewport();

        /** Returns main camera */
        static std::shared_ptr<Camera> getMainCamera();

        /** Sets main camera */
        static void setMainCamera(const std::shared_ptr<Camera>& camera);

        /** Add child node into root node of scene */
        static void addSceneNode(const std::shared_ptr<SceneNode>& node);

        /** Clear back buffer and render scene */
        static void renderScene();

        /** Present back buffer into screen */
        static void presentBackBuffer();
    };
}

#endif