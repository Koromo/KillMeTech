#include "scene.h"
#include "camera.h"
#include "light.h"
#include "meshinstance.h"
#include "mesh.h"
#include "material.h"
#include "effecttechnique.h"
#include "effectpass.h"
#include "../renderer/rendersystem.h"
#include "../renderer/renderdevice.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/rendertarget.h"
#include "../renderer/depthstencil.h"
#include "../renderer/vertexdata.h"
#include "../renderer/commandlist.h"
#include "../renderer/commandqueue.h"
#include "../core/math/matrix44.h"
#include <cassert>

namespace killme
{
    Scene::Scene(RenderSystem& renderSystem)
        : device_(renderSystem.getDevice())
        , scissorRect_()
        , ambientLight_(0.2f, 0.2f, 0.2f, 1)
        , dirLights_()
        , pointLights_()
        , cameras_()
        , meshInstances_()
        , mainCamera_()
    {
        const auto window = renderSystem.getTargetWindow();
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
        if (light->getType() == LightType::directional)
        {
            dirLights_.emplace(light);
        }
        else
        {
            pointLights_.emplace(light);
        }
    }

    void Scene::removeLight(const std::shared_ptr<Light>& light)
    {
        if (light->getType() == LightType::directional)
        {
            dirLights_.erase(light);
        }
        else
        {
            pointLights_.erase(light);
        }
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

                // For each passes
                for (const auto& pass : material->getUseTechnique()->getPasses())
                {
                    const auto pipeline = pass->getPipelineState();

                    const auto renderPass = [&]()
                    {
                        pipeline->setRenderTarget(0, frame.backBufferLocation);
                        pipeline->setDepthStencil(frame.depthStencilLocation);
                        pipeline->setViewport(viewport);
                        pipeline->setScissorRect(scissorRect_);
                        pipeline->setPrimitiveTopology(PrimitiveTopology::triangeList);
                        pipeline->setVertexBuffers(vertexData);

                        // Add draw commands
                        const auto allocator = device_->obtainCommandAllocator();
                        const auto commands = device_->obtainCommandList(allocator, pipeline);

                        commands->transitionBarrior(frame.backBuffer, GpuResourceState::present, GpuResourceState::renderTarget);
                        commands->drawIndexed(vertexData->getIndexBuffer()->getNumIndices());
                        commands->transitionBarrior(frame.backBuffer, GpuResourceState::renderTarget, GpuResourceState::present);

                        commands->close();

                        const auto commandExe = { commands };
                        device_->getCommandQueue()->executeCommands(commandExe);

                        device_->reuseCommandAllocatorAfterExecution(allocator);
                        device_->reuseCommandListAfterExecution(commands);
                    };

                    if (pass->getLightIteration() == LightIteration::directional)
                    {
                        for (const auto& light : dirLights_)
                        {
                            const auto lightColor = light->getColor();
                            const auto lightDir = light->getDirection();
                            pass->updateConstant("_LightColor", &lightColor, sizeof(lightColor));
                            pass->updateConstant("_LightDirection", &lightDir, sizeof(lightDir));
                            renderPass();
                        }
                    }
                    else if (pass->getLightIteration() == LightIteration::point)
                    {
                        for (const auto& light : pointLights_)
                        {
                            const auto lightColor = light->getColor();
                            const auto lightPos = light->getPosition();
                            const auto lightAttRange = light->getAttenuationRange();
                            const auto lightAttConstant = light->getAttenuationConstant();
                            const auto lightAttLiner = light->getAttenuationLiner();
                            const auto lightAttQuadratic = light->getAttenuationQuadratic();
                            pass->updateConstant("_LightColor", &lightColor, sizeof(lightColor));
                            pass->updateConstant("_LightPosition", &lightPos, sizeof(lightPos));
                            pass->updateConstant("_LightAttRange", &lightAttRange, sizeof(lightAttRange));
                            pass->updateConstant("_LightAttConstant", &lightAttConstant, sizeof(lightAttConstant));
                            pass->updateConstant("_LightAttLiner", &lightAttLiner, sizeof(lightAttLiner));
                            pass->updateConstant("_LightAttQuadratic", &lightAttQuadratic, sizeof(lightAttQuadratic));
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

        device_->getCommandQueue()->waitForCommands();
    }
}
