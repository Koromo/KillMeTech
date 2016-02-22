#ifndef _KILLME_SCENE_H_
#define _KILLME_SCENE_H_

#include "../renderer/rendersystem.h"
#include "../renderer/renderstate.h"
#include "../core/math/matrix44.h"
#include "../core/math/color.h"
#include "../core/utility.h"
#include <memory>
#include <unordered_set>
#include <type_traits>

namespace killme
{
    class RenderSystem;
    class SceneNode;
    class CommandList;
    class ConstantBuffer;
    class Camera;
    class Light;

    struct SceneContext
    {
        FrameResource frame;
        Matrix44 viewMatrix;
        Matrix44 projMatrix;
        Viewport viewport;
        Color ambientLight;
        Range_t<std::add_lvalue_reference_t<std::unordered_set<std::shared_ptr<Light>>>> lights_;
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
        Color ambientLight_;
        std::unordered_set<std::shared_ptr<Light>> lights_;

    public:
        /** Constructs */
        explicit Scene(const std::shared_ptr<RenderSystem>& renderSystem);

        /** Returns the current scene */
        std::shared_ptr<SceneNode> getRootNode();

        /** Set ambient light */
        void setAmbientLight(const Color& c);

        /** Add light */
        void addLight(const std::shared_ptr<Light>& light);

        /** Remove light */
        void removeLight(const std::shared_ptr<Light>& light);

        /** Clears the back buffer and draws the current scene */
        void renderScene(const Camera& camera);
    };
}

#endif
