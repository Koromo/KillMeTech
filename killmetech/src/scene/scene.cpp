#include "scene.h"
#include "scenenode.h"
#include "camera.h"
#include "renderqueue.h"
#include "../renderer/rendersystem.h"
#include "../renderer/commandlist.h"
#include "../core/math/matrix44.h"
#include "../core/math/color.h"
#include <Windows.h>

namespace killme
{
    Scene::Scene(const std::shared_ptr<RenderSystem>& renderSystem)
        : renderSystem_(renderSystem)
        , commandList_()
        , rootNode_(std::make_shared<SceneNode>())
        , ambientLight_(0.2f, 0.2f, 0.2f, 1)
        , lights_()
    {
        commandList_ = renderSystem_->createCommandList();
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

    void Scene::renderScene(const Camera& camera)
    {
        // Clear the render target and the depth stencil
        const auto frame = renderSystem_->getCurrentFrameResource();

        renderSystem_->beginCommands(commandList_, nullptr);
        commandList_->resourceBarrior(frame.backBuffer, ResourceState::present, ResourceState::renderTarget);
        commandList_->clearRenderTarget(frame.backBufferView, { 0.1f, 0.1f, 0.1f, 1 });
        commandList_->resourceBarrior(frame.backBuffer, ResourceState::renderTarget, ResourceState::present);
        commandList_->clearDepthStencil(frame.depthStencilView, 1);
        commandList_->endCommands();
        renderSystem_->executeCommands(commandList_);

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
        context.commandList = commandList_;
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
            queue.pop()->recordCommands(context);
            renderSystem_->executeCommands(context.commandList);
        }
    }
}
