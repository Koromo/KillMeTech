#ifndef _KILLME_SCENE_H_
#define _KILLME_SCENE_H_

#include "../renderer/rendersystem.h"
#include "../renderer/renderstate.h"
#include "../core/math/matrix44.h"
#include <memory>

namespace killme
{
    class RenderSystem;
    class SceneNode;
    class CommandList;
    class ConstantBuffer;
    class Camera;

    struct SceneContext
    {
        FrameResource frame;
        Matrix44 viewMatrix;
        Matrix44 projMatrix;
        Viewport viewport;
        ScissorRect scissorRect;
        std::shared_ptr<CommandList> commandList;
        std::shared_ptr<RenderSystem> renderSystem;
    };

    /** The render scene */
    class Scene
    {
    private:
        std::shared_ptr<RenderSystem> renderSystem_;
        std::shared_ptr<CommandList> commandList_;
        std::shared_ptr<SceneNode> rootNode_;

    public:
        /** Constructs */
        explicit Scene(const std::shared_ptr<RenderSystem>& renderSystem);

        /** Returns the current scene */
        std::shared_ptr<SceneNode> getRootNode();

        /** Clears the back buffer and draws the current scene */
        void renderScene(const Camera& camera);
    };
}

#endif
