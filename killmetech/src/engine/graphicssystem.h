#ifndef _KILLME_GRAPHICSSYSTEM_H_
#define _KILLME_GRAPHICSSYSTEM_H_

#include "../renderer/rendersystem.h"
#include "../renderer/renderstate.h"
#include <Windows.h>
#include <memory>

namespace killme
{
    class RenderSystem;

    /** Graphics subsystem */
    class GraphicsSystem
    {
    private:
        std::shared_ptr<RenderSystem> renderSystem_;
        Viewport clientViewport_;

    public:
        /** Initialize the graphics subsystem */
        void startup(HWND window);

        /** Finalize the graphics subsystem */
        void shutdown();

        /** Return the render system */
        std::shared_ptr<RenderSystem> getRenderSystem();

        /** Return current frame resource */
        FrameResource getCurrentFrameResource();

        /** Return a viewport based on client size of render window */
        Viewport getClientViewport() const;

        /** Clear current back buffer */
        void clearBackBuffer();

        /** Present current back buffer into screen */
        void presentBackBuffer();
    };

    extern GraphicsSystem graphicsSystem;
}

#endif