#ifndef _KILLME_SCENEMANAGER_H_
#define _KILLME_SCENEMANAGER_H_

#include "../renderer/renderstate.h"
#include <Windows.h>
#include <memory>

namespace killme
{
    class Material;
    class SceneNode;
    class CommandList;
    class ConstantBuffer;
    class GpuResourceHeap;

    /** Scene manager */
    class SceneManager
    {
    private:
        std::shared_ptr<CommandList> commandList_;
        std::shared_ptr<ConstantBuffer> transformBuffer_;
        Viewport viewport_;
        ScissorRect scissorRect_;
        std::shared_ptr<SceneNode> rootNode_;

    public:
        /** Initializes */
        void startup();

        /** Finalizes */
        void shutdown();

        /** Returns current scene */
        std::shared_ptr<SceneNode> getRootNode();

        /** Draw scene */
        void drawScene();

        /** Swap screen */
        void presentBackBuffer();

        /** Set scene resource heaps */
        void setSceneResourceHeaps(Material& m);
    };

    extern SceneManager sceneManager;
}

#endif
