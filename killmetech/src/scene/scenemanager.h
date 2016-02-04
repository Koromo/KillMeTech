#ifndef _KILLME_SCENEMANAGER_H_
#define _KILLME_SCENEMANAGER_H_

#include "../renderer/renderstate.h"
#include <Windows.h>
#include <memory>

namespace killme
{
    class SceneNode;
    class RenderSystem;
    class CommandList;
    class ConstantBuffer;
    class GpuResourceHeap;

    /** Scene manager */
    class SceneManager
    {
    private:
        std::shared_ptr<RenderSystem> renderSystem_;
        std::shared_ptr<CommandList> commandList_;
        Viewport viewport_;
        ScissorRect scissorRect_;
        std::shared_ptr<ConstantBuffer> viewProjMatBuffer_;
        std::shared_ptr<ConstantBuffer> worldMatBuffer_;
        std::shared_ptr<GpuResourceHeap> transMatrixHeap_;
        std::shared_ptr<SceneNode> rootNode_;

    public:
        /** Construct */
        explicit SceneManager(HWND window);

        /** Returns render system */
        std::shared_ptr<RenderSystem> getRenderSystem();

        /** Returns current scene */
        std::shared_ptr<SceneNode> getRootNode();

        /** Draw scene */
        void drawScene();

        /** Swap screen */
        void presentBackBuffer();
    };
}

#endif
