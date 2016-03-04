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
        Range<std::add_lvalue_reference_t<std::unordered_set<std::shared_ptr<Light>>>> lights_;
        ScissorRect scissorRect;
        std::shared_ptr<RenderSystem> renderSystem;

    };

    /** Render scene */
    class Scene
    {
    private:
        std::shared_ptr<RenderSystem> renderSystem_;
        std::shared_ptr<SceneNode> rootNode_;
        Color ambientLight_;
        std::unordered_set<std::shared_ptr<Light>> lights_;

    public:
        /** Construct */
        explicit Scene(const std::shared_ptr<RenderSystem>& renderSystem);

        /** Return the current scene */
        std::shared_ptr<SceneNode> getRootNode();

        /** Set ambient light */
        void setAmbientLight(const Color& c);

        /** Add light */
        void addLight(const std::shared_ptr<Light>& light);

        /** Remove light */
        void removeLight(const std::shared_ptr<Light>& light);

        /** Clear the back buffer and draw the current scene */
        void renderScene(const Camera& camera, const FrameResource& frame);
    };
}

#endif
