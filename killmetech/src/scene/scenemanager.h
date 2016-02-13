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
    class FbxMeshImporter;

    /** The scene manager */
    class SceneManager
    {
    private:
        std::shared_ptr<CommandList> commandList_;
        std::shared_ptr<ConstantBuffer> transformBuffer_;
        ScissorRect scissorRect_;
        std::unique_ptr<FbxMeshImporter> importManager_;
        std::shared_ptr<SceneNode> rootNode_;

    public:
        /** Initializes */
        void startup();

        /** Finalizes */
        void shutdown();

        /** Returns the current scene */
        std::shared_ptr<SceneNode> getRootNode();

        /** Clears the back buffer and draws the current scene */
        void drawScene();

        /** Swaps the screen */
        void presentBackBuffer();

        /** Sets scene resource heaps into a material */
        void setSceneResourceHeaps(Material& m);
    };

    extern SceneManager sceneManager;
}

#endif
