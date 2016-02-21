#include "meshentity.h"
#include "mesh.h"
#include "material.h"
#include "effecttechnique.h"
#include "effectpass.h"
#include "renderqueue.h"
#include "scene.h"
#include "scenenode.h"
#include "../renderer/rendersystem.h"
#include "../renderer/rendertarget.h"
#include "../renderer/commandlist.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/vertexshader.h"
#include "../renderer/vertexdata.h"

namespace killme
{
    MeshEntity::MeshEntity(const Resource<Mesh>& mesh)
        : mesh_()
        , renderer_(std::make_shared<MeshRenderer>(mesh))
    {
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

    void MeshRenderer::recordCommands(const SceneContext& context)
    {
        // Get world matrix
        const auto worldMatrix = transpose(node_.lock()->getWorldMatrix());

        for (const auto& subMesh : mesh_.access()->getSubMeshes()) // For each sub meshes
        {
            // Update constant buffers
            const auto material = subMesh.second->getMaterial();
            material.access()->setVariable("_ViewMatrix", context.viewMatrix);
            material.access()->setVariable("_ProjMatrix", context.projMatrix);
            material.access()->setVariable("_WorldMatrix", worldMatrix);

            const auto vertexData = subMesh.second->getVertexData();
            const auto indexBuffer = vertexData->getIndexBuffer();
            for (const auto& pass : material.access()->getUseTechnique()->getPasses()) // For each passes
            {
                const auto pipelineState = pass.second->getPipelineState();
                const auto rootSignature = pipelineState->describe().rootSignature;
                const auto inputLayout = pipelineState->describe().vertexShader.access()->getInputLayout();
                const auto& vertexBinder = vertexData->getBinder(inputLayout);
                const auto heaps = pass.second->getGpuResourceHeaps();
                const auto heapTables = pass.second->getGpuResourceHeapTables();

                // Add draw commands
                context.renderSystem->beginCommands(context.commandList, pipelineState);

                context.commandList->resourceBarrior(context.frame.backBuffer, ResourceState::present, ResourceState::renderTarget);
                context.commandList->setRenderTarget(context.frame.backBufferView, context.frame.depthStencilView);
                context.commandList->setViewport(context.viewport);
                context.commandList->setScissorRect(context.scissorRect);
                context.commandList->setPrimitiveTopology(PrimitiveTopology::triangeList);
                context.commandList->setVertexBuffers(vertexBinder);
                context.commandList->setIndexBuffer(indexBuffer);

                context.commandList->setRootSignature(rootSignature);
                context.commandList->setGpuResourceHeaps(heaps, heaps.length());
                for (const auto& t : heapTables)
                {
                    context.commandList->setGpuResourceTable(t.first, t.second);
                }

                context.commandList->drawIndexed(indexBuffer->getNumIndices());
                context.commandList->resourceBarrior(context.frame.backBuffer, ResourceState::renderTarget, ResourceState::present);

                context.commandList->endCommands();

                context.renderSystem->executeCommands(context.commandList);
            }
        }
    }
}