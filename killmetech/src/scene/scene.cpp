#include "scene.h"
#include "camera.h"
#include "light.h"
#include "meshinstance.h"
#include "mesh.h"
#include "material.h"
#include "effecttechnique.h"
#include "effectpass.h"
#include "../renderer/rendersystem.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/shaders.h"
#include "../renderer/vertexdata.h"
#include "../renderer/commandlist.h"
#include "../renderer/rendertarget.h"
#include "../resources/resource.h"
#include "../core/math/matrix44.h"
#include <Windows.h>
#include <cassert>

namespace killme
{
    Scene::Scene(const std::shared_ptr<RenderSystem>& renderSystem)
        : renderSystem_(renderSystem)
        , scissorRect_()
        , ambientLight_(0.2f, 0.2f, 0.2f, 1)
        , lights_()
        , cameras_()
        , meshInstances_()
        , mainCamera_()
    {
        const auto window = renderSystem_->getTargetWindow();
        RECT clientRect;
        GetClientRect(window, &clientRect);

        scissorRect_.top = 0;
        scissorRect_.left = 0;
        scissorRect_.right = clientRect.right - clientRect.left;
        scissorRect_.bottom = clientRect.bottom - clientRect.top;
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

    void Scene::addCamera(const std::shared_ptr<Camera>& camera)
    {
        cameras_.emplace(camera);
    }

    void Scene::removeCamera(const std::shared_ptr<Camera>& camera)
    {
        cameras_.erase(camera);
        if (mainCamera_ == camera)
        {
            mainCamera_.reset();
        }
    }

    void Scene::setMainCamera(const std::shared_ptr<Camera>& camera)
    {
        assert((!camera || cameras_.find(camera) != std::cend(cameras_)) && "This camera is not exists in this scene");
        mainCamera_ = camera;
    }

    std::shared_ptr<Camera> Scene::getMainCamera()
    {
        return mainCamera_;
    }

    void Scene::addMeshInstance(const std::shared_ptr<MeshInstance>& inst)
    {
        meshInstances_.emplace(inst);
    }

    void Scene::removeMeshInstance(const std::shared_ptr<MeshInstance>& inst)
    {
        meshInstances_.erase(inst);
    }

    void Scene::renderScene(const FrameResource& frame)
    {
        if (!mainCamera_)
        {
            return;
        }

        // Get parameters of camera
        const auto viewMatrix = transpose(mainCamera_->getViewMatrix());
        const auto projMatrix = transpose(mainCamera_->getProjectionMatrix());
        const auto viewport = mainCamera_->getViewport();

        // For each mesh instances
        for (const auto& inst : meshInstances_)
        {
            // Get instance parameters
            const auto worldMatrix = transpose(inst->getWorldMatrix());
            const auto mesh = inst->getMesh();

            // For each submeshes
            for (const auto& submesh : mesh.access()->getSubmeshes())
            {
                // Update constant buffers
                const auto material = submesh.second->getMaterial();
                material->setNumeric("_ViewMatrix", to<MP_float4x4>(viewMatrix));
                material->setNumeric("_ProjMatrix", to<MP_float4x4>(projMatrix));
                material->setNumeric("_WorldMatrix", to<MP_float4x4>(worldMatrix));
                material->setNumeric("_AmbientLight", to<MP_float4>(ambientLight_));

                // Get render resources
                const auto vertexData = submesh.second->getVertexData();
                const auto indexBuffer = vertexData->getIndexBuffer();

                // For each passes
                for (const auto& pass : material->getUseTechnique()->getPasses())
                {
                    const auto pipelineState = pass->getPipelineState();
                    const auto rootSignature = pipelineState->describe().rootSignature;
                    const auto inputLayout = pipelineState->describe().vertexShader.access()->getInputLayout();
                    const auto& vertexViews = vertexData->getVertexViews(inputLayout);
                    const auto heaps = pass->getGpuResourceHeaps();
                    const auto heapTables = pass->getGpuResourceHeapTables();

                    const auto renderPass = [&]()
                    {
                        // Add draw commands
                        const auto commands = renderSystem_->beginCommands(pipelineState);

                        commands->resourceBarrior(frame.backBuffer, ResourceState::present, ResourceState::renderTarget);
                        commands->setRenderTarget(frame.backBufferView, frame.depthStencilView);
                        commands->setViewport(viewport);
                        commands->setScissorRect(scissorRect_);
                        commands->setPrimitiveTopology(PrimitiveTopology::triangeList);
                        commands->setVertexBuffers(vertexViews);
                        commands->setIndexBuffer(indexBuffer);

                        commands->setRootSignature(rootSignature);
                        commands->setGpuResourceHeaps(heaps);
                        for (const auto& t : heapTables)
                        {
                            commands->setGpuResourceTable(t.first, t.second);
                        }

                        commands->drawIndexed(indexBuffer->getNumIndices());
                        commands->resourceBarrior(frame.backBuffer, ResourceState::renderTarget, ResourceState::present);

                        commands->close();

                        renderSystem_->executeCommands(commands);
                    };

                    if (pass->forEachLight())
                    {
                        for (const auto& light : lights_)
                        {
                            const auto lightColor = light->getColor();
                            const auto lightDir = light->getFront();
                            pass->updateConstant("_LightColor", &lightColor, sizeof(lightColor));
                            pass->updateConstant("_LightDirection", &lightDir, sizeof(lightDir));
                            renderPass();
                        }
                    }
                    else
                    {
                        renderPass();
                    }
                }
            }
        }
    }
}
