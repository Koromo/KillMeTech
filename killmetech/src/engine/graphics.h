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

    struct Graphics
    {
        static void startup(HWND window);
        static void shutdown();

        static std::shared_ptr<RenderSystem> getRenderSystem();

        static Viewport getClientViewport();

        static std::shared_ptr<Camera> getMainCamera();
        static void setMainCamera(const std::shared_ptr<Camera>& camera);
        static void addSceneNode(const std::shared_ptr<SceneNode>& node);

        static void renderScene();
        static void presentBackBuffer();
    };
}

#endif