#include "meshentity.h"
#include "mesh.h"
#include "material.h"
#include "effecttechnique.h"
#include "effectpass.h"
#include "renderqueue.h"
#include "scene.h"
#include "scenenode.h"
#include "light.h"
#include "../renderer/rendersystem.h"
#include "../renderer/rendertarget.h"
#include "../renderer/commandlist.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/vertexshader.h"
#include "../renderer/vertexdata.h"

namespace killme
{
    MeshEntity::MeshEntity(const Resource<Mesh>& mesh)
        : mesh_(mesh)
        , renderer_(std::make_shared<MeshRenderer>(mesh))
    {
    }

    std::shared_ptr<SubMesh> MeshEntity::findSubMesh(const std::string& name)
    {
        return mesh_.access()->findSubMesh(name);
    }

    void MeshEntity::collectRenderer(RenderQueue& queue)
    {
        renderer_->setOwnerNode(getOwner());
        queue.push(renderer_);
    }

    MeshRenderer::MeshRenderer(const Resource<Mesh>& mesh)
        : node_()
        , mesh_(mesh)
    {
    }

    void MeshRenderer::setOwnerNode(const std::weak_ptr<SceneNode>& node)
    {
        node_ = node;
    }

    void MeshRenderer::render(const SceneContext& context)
    {
        // Get world matrix
        const auto worldMatrix = transpose(node_.lock()->getWorldMatrix());

        for (const auto& subMesh : mesh_.access()->getSubMeshes()) // For each sub meshes
        {
            // Update constant buffers
            const auto material = subMesh.second->getMaterial();
            material->setNumeric("_ViewMatrix", to<MP_float4x4>(context.viewMatrix));
            material->setNumeric("_ProjMatrix", to<MP_float4x4>(context.projMatrix));
            material->setNumeric("_WorldMatrix", to<MP_float4x4>(worldMatrix));
            material->setNumeric("_AmbientLight", to<MP_float4>(context.ambientLight));

            const auto vertexData = subMesh.second->getVertexData();
            const auto indexBuffer = vertexData->getIndexBuffer();
            for (const auto& pass : material->getUseTechnique()->getPasses()) // For each passes
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
                    context.renderSystem->beginCommands(context.commandList, pipelineState);

                    context.commandList->resourceBarrior(context.frame.backBuffer, ResourceState::present, ResourceState::renderTarget);
                    context.commandList->setRenderTarget(context.frame.backBufferView, context.frame.depthStencilView);
                    context.commandList->setViewport(context.viewport);
                    context.commandList->setScissorRect(context.scissorRect);
                    context.commandList->setPrimitiveTopology(PrimitiveTopology::triangeList);
                    context.commandList->setVertexBuffers(vertexViews);
                    context.commandList->setIndexBuffer(indexBuffer);

                    context.commandList->setRootSignature(rootSignature);
                    context.commandList->setGpuResourceHeaps(heaps);
                    for (const auto& t : heapTables)
                    {
                        context.commandList->setGpuResourceTable(t.first, t.second);
                    }

                    context.commandList->drawIndexed(indexBuffer->getNumIndices());
                    context.commandList->resourceBarrior(context.frame.backBuffer, ResourceState::renderTarget, ResourceState::present);

                    context.commandList->endCommands();

                    context.renderSystem->executeCommands(context.commandList);
                };

                if (pass->forEachLight())
                {
                    for (const auto& light : context.lights_)
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