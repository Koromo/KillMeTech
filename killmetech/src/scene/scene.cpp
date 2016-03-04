#include "scene.h"
#include "scenenode.h"
#include "camera.h"
#include "renderqueue.h"
#include "../renderer/rendersystem.h"
#include <Windows.h>

namespace killme
{
    Scene::Scene(const std::shared_ptr<RenderSystem>& renderSystem)
        : renderSystem_(renderSystem)
        , rootNode_(std::make_shared<SceneNode>())
        , ambientLight_(0.2f, 0.2f, 0.2f, 1)
        , lights_()
    {
    }

    std::shared_ptr<SceneNode> Scene::getRootNode()
    {
        return rootNode_;
    }

    void Scene::setAmbientLight(const Color& c)
    {
        ambientLight_ = c;
    }

    void Scene::addLight(const std::shared_ptr<Light>& light)
    {
        lights_.emplace(light);
    }

    void Scene::removeLight(const std::shared_ptr<Light>& light)
    {
        lights_.erase(light);
    }

    void Scene::renderScene(const Camera& camera, const FrameResource& frame)
    {
        // Collect render objects
        RenderQueue queue;
        rootNode_->collectRenderer(queue);

        // Create scene context
        const auto window = renderSystem_->getTargetWindow();
        RECT clientRect;
        GetClientRect(window, &clientRect);

        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        SceneContext context;
        context.frame = frame;
        context.renderSystem = renderSystem_;

        context.scissorRect.top = 0;
        context.scissorRect.left = 0;
        context.scissorRect.right = clientWidth;
        context.scissorRect.bottom = clientHeight;

        context.viewMatrix = transpose(camera.getViewMatrix());
        context.projMatrix = transpose(camera.getProjectionMatrix());
        context.viewport = camera.getViewport();
        context.ambientLight = ambientLight_;
        context.lights_ = makeRange(lights_);

        // Render
        while (!queue.empty())
        {
            queue.pop()->render(context);
        }
    }
}
