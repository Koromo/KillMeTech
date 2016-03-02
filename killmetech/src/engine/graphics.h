#ifndef _KILLME_GRAPHICS_H_
#define _KILLME_GRAPHICS_H_

#include <Windows.h>
#include <memory>

namespace killme
{
    class Scene;
    class SceneNode;
    class Camera;
    class Light;
    class RenderSystem;
    class Color;
    struct Viewport;

    /** Rendering subsystem */
    struct Graphics
    {
        /** Startup rendering subsystem */
        static void startup(HWND window);

        /** Shutdown rendering subsystem */
        static void shutdown();

        /** Set amgient light */
        static void setAmbientLight(const Color& c);

        /** Set a light */
        static void addLight(const std::shared_ptr<Light>& light);

        /** Remove a light */
        static void removeLight(const std::shared_ptr<Light>& light);

        /** Return render system */
        static std::shared_ptr<RenderSystem> getRenderSystem();

        /** Return viewport based on client size of render window */
        static Viewport getClientViewport();

        /** Return main camera */
        static std::shared_ptr<Camera> getMainCamera();

        /** Set main camera */
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