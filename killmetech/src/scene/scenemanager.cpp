#include "scenemanager.h"
#include "scenenode.h"
#include "scenevisitor.h"
#include "camera.h"
#include "meshentity.h"
#include "mesh.h"
#include "material.h"
#include "../renderer/rendersystem.h"
#include "../renderer/constantbuffer.h"
#include "../renderer/commandlist.h"
#include "../renderer/rendertarget.h"
#include "../renderer/vertexdata.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/vertexshader.h"
#include "../core/math/matrix44.h"
#include "../core/math/color.h"
#include <vector>

namespace killme
{
    SceneManager sceneManager;

    void SceneManager::startup()
    {
        // Create render resources
        commandList_ = renderSystem.createCommandList();
        transformBuffer_ = renderSystem.createConstantBuffer(sizeof(Matrix44) * 3);

        RECT clientRect;
        GetClientRect(renderSystem.getTargetWindow(), &clientRect);
        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        scissorRect_.top = 0;
        scissorRect_.left = 0;
        scissorRect_.right = static_cast<int>(clientWidth);
        scissorRect_.bottom = static_cast<int>(clientHeight);

        // Create the root scene node
        rootNode_ = std::make_shared<SceneNode>(std::shared_ptr<SceneNode>());
    }

    void SceneManager::shutdown()
    {
        rootNode_.reset();
        transformBuffer_.reset();
        commandList_.reset();
    }

    std::shared_ptr<SceneNode> SceneManager::getRootNode()
    {
        return rootNode_;
    }

    namespace
    {
        struct CollectRenderObjects : SceneVisitor
        {
            std::shared_ptr<Camera> camera;
            std::vector<std::shared_ptr<MeshEntity>> entities;

            bool operator()(const std::shared_ptr<SceneNode>& n, const std::shared_ptr<Camera>& c)
            {
                camera = c;
                return false;
            }

            bool operator()(const std::shared_ptr<SceneNode>& n, const std::shared_ptr<MeshEntity>& e)
            {
                entities.push_back(e);
                return false;
            }
        };
    }

    void SceneManager::drawScene()
    {
        // Clear the render target and the depth stencil
        renderSystem.resetCommandList(commandList_, nullptr);

        const auto renderTarget = renderSystem.getCurrentBackBuffer();
        const auto depthStencil = renderSystem.getDepthStencil();
        commandList_->resourceBarrior(renderTarget, ResourceState::present, ResourceState::renderTarget);
        commandList_->clearRenderTarget(renderTarget, { 0.1f, 0.1f, 0.1f, 1 });
        commandList_->resourceBarrior(renderTarget, ResourceState::renderTarget, ResourceState::present);
        commandList_->clearDepthStencil(depthStencil, 1);
        commandList_->close();
        renderSystem.executeCommandList(commandList_);

        // Collect render objects
        CollectRenderObjects visitor;
        rootNode_->depthTraverse(visitor);

        // Update the constant buffer about view and projection matrix
        const auto camera = visitor.camera;
        if (!camera)
        {
            return;
        }

        const auto viewMatrix = transpose(inverse(camera->lockOwner()->getWorldMatrix()));
        const auto projMatrix = transpose(camera->getProjectionMatrix());

        transformBuffer_->update(&viewMatrix, sizeof(Matrix44), sizeof(Matrix44));
        transformBuffer_->update(&projMatrix, sizeof(Matrix44) * 2, sizeof(Matrix44));

        // For each mesh entities
        const auto viewport = camera->getViewport();

        for (const auto& entity : visitor.entities)
        {
            // Update the constant buffer about world matrix
            const auto worldMatrix = transpose(entity->lockOwner()->getWorldMatrix());
            transformBuffer_->update(&worldMatrix, 0, sizeof(Matrix44));

            // Get render resources
            const auto mesh = entity->getMesh();
            const auto material = mesh->getMaterial();

            const auto vertexData = mesh->getVertexData();
            const auto pipelineState = material->getPipelineState();
            const auto rootSignature = pipelineState->describe().rootSignature;
            const auto inputLayout = pipelineState->describe().vertexShader.access()->getInputLayout();
            const auto& vertexBinder = vertexData->getBinder(inputLayout);
            const auto indexBuffer = vertexData->getIndexBuffer();
            const auto heaps = { material->getConstantBufferHeap() };
            const auto heapTables = material->getConstantBufferHeapTables();

            // Add draw commands
            renderSystem.resetCommandList(commandList_, pipelineState);

            commandList_->resourceBarrior(renderTarget, ResourceState::present, ResourceState::renderTarget);
            commandList_->setRenderTarget(renderTarget, depthStencil);
            commandList_->setViewport(viewport);
            commandList_->setScissorRect(scissorRect_);
            commandList_->setPrimitiveTopology(PrimitiveTopology::triangeList);
            commandList_->setVertexBuffers(vertexBinder);
            commandList_->setIndexBuffer(indexBuffer);

            commandList_->setRootSignature(rootSignature);
            commandList_->setGpuResourceHeaps(heaps, 1);
            for (const auto& t : heapTables)
            {
                commandList_->setGpuResourceTable(t.first, t.second);
            }

            commandList_->drawIndexed(indexBuffer->getNumIndices());
            commandList_->resourceBarrior(renderTarget, ResourceState::renderTarget, ResourceState::present);

            commandList_->close();

            // Draw entity
            renderSystem.executeCommandList(commandList_);
        }
    }

    void SceneManager::presentBackBuffer()
    {
        renderSystem.presentBackBuffer();
    }

    void SceneManager::setSceneResourceHeaps(Material& m)
    {
        m.storeConstantBuffer("Transform", transformBuffer_);
    }
}
