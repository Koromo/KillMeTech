#include "scenemanager.h"
#include "scenenode.h"
#include "scenevisitor.h"
#include "camera.h"
#include "meshentity.h"
#include "mesh.h"
#include "material.h"
#include "../renderer/rendersystem.h"
#include "../renderer/resourceheap.h"
#include "../renderer/constantbuffer.h"
#include "../renderer/commandlist.h"
#include "../renderer/rendertarget.h"
#include "../renderer/renderstate.h"
#include "../renderer/vertexdata.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/vertexshader.h"
#include "../core/range.h"
#include "../core/math/matrix44.h"
#include "../core/math/color.h"
#include <vector>

namespace killme
{
    namespace
    {
        struct ViewProjMatData
        {
            Matrix44 viewMatrix;
            Matrix44 projMatrix;
        };


        /**
         *  vs0: ViewProjMat
         *  vs1: WorldMat
         *  ps0: Material
        */
    }

    SceneManager::SceneManager(HWND window)
        : renderSystem_(std::make_shared<RenderSystem>(window))
        , commandList_()
        , viewport_()
        , scissorRect_()
        , viewProjMatBuffer_()
        , worldMatBuffer_()
        , transMatrixHeap_()
        , rootNode_(std::make_shared<SceneNode>(nullptr))
    {
        commandList_ = renderSystem_->createCommandList();
        viewProjMatBuffer_ = renderSystem_->createConstantBuffer(sizeof(ViewProjMatData));
        worldMatBuffer_ = renderSystem_->createConstantBuffer(sizeof(Matrix44));
        transMatrixHeap_ = renderSystem_->createResourceHeap(2, ResourceHeapType::constantBuffer, ResourceHeapFlag::shaderVisible);
        renderSystem_->storeResource(transMatrixHeap_, 0, viewProjMatBuffer_);
        renderSystem_->storeResource(transMatrixHeap_, 1, worldMatBuffer_);

        RECT clientRect;
        GetClientRect(window, &clientRect);
        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        viewport_.width = static_cast<float>(clientWidth);
        viewport_.height = static_cast<float>(clientHeight);
        viewport_.topLeftX = 0;
        viewport_.topLeftY = 0;
        viewport_.minDepth = 0;
        viewport_.maxDepth = 1;

        scissorRect_.top = 0;
        scissorRect_.left = 0;
        scissorRect_.right = static_cast<int>(clientWidth);
        scissorRect_.bottom = static_cast<int>(clientHeight);
    }

    std::shared_ptr<RenderSystem> SceneManager::getRenderSystem()
    {
        return renderSystem_;
    }

    std::shared_ptr<SceneNode> SceneManager::getRootNode()
    {
        return rootNode_;
    }

    namespace
    {
        struct DrawVisitor : SceneVisitor
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
        // Traverse scene
        DrawVisitor visitor;
        rootNode_->depthTraverse(visitor);

        // Update constant buffer of view matrix and projection matrix
        const auto camera = visitor.camera;
        const auto viewMatrix = transpose(inverse(camera->lockOwner()->getWorldMatrix()));
        const auto projMatrix = transpose(camera->getProjectionMatrix());
        
        ViewProjMatData viewProjMatData = { viewMatrix, projMatrix };
        viewProjMatBuffer_->update(&viewProjMatData);

        // Clear render target
        renderSystem_->resetCommandList(commandList_, nullptr);

        const auto renderTarget = renderSystem_->getCurrentBackBuffer();
        const auto depthStencil = renderSystem_->getDepthStencil();
        commandList_->resourceBarrior(renderTarget, ResourceState::present, ResourceState::renderTarget);
        commandList_->clearRenderTarget(renderTarget, { 0.1f, 0.1f, 0.1f, 1 });
        commandList_->resourceBarrior(renderTarget, ResourceState::renderTarget, ResourceState::present);
        commandList_->clearDepthStencil(depthStencil, 1);
        commandList_->close();
        renderSystem_->executeCommandList(commandList_);

        // For each mesh entities
        for (const auto& entity : visitor.entities)
        {
            // Update constant buffer of world matrix
            const auto worldMatrix = transpose(entity->lockOwner()->getWorldMatrix());
            worldMatBuffer_->update(&worldMatrix);

            const auto mesh = entity->getMesh();
            const auto material = mesh->getMaterial();

            const auto vertexData = mesh->getVertexData();
            const auto pipelineState = material->getPipelineState();
            const auto rootSignature = pipelineState->describe().rootSignature;
            const auto inputLayout = pipelineState->describe().vertexShader->getInputLayout();
            const auto vertexBinder = vertexData->getBinder(inputLayout);
            const auto indexBuffer = vertexData->getIndexBuffer();

            // Add draw commands
            renderSystem_->resetCommandList(commandList_, pipelineState);

            commandList_->resourceBarrior(renderTarget, ResourceState::present, ResourceState::renderTarget);
            commandList_->setRenderTarget(renderTarget, depthStencil);
            commandList_->setViewport(viewport_);
            commandList_->setScissorRect(scissorRect_);
            commandList_->setPrimitiveTopology(PrimitiveTopology::triangeList);
            commandList_->setVertexBuffers(vertexBinder);
            commandList_->setIndexBuffer(indexBuffer);
            commandList_->setRootSignature(rootSignature);

            const auto heaps = { transMatrixHeap_ };
            commandList_->setResourceHeaps(makeRange(heaps), 1);
            commandList_->setResourceTable(0, transMatrixHeap_);

            commandList_->drawIndexed(indexBuffer->getNumIndices());
            commandList_->resourceBarrior(renderTarget, ResourceState::renderTarget, ResourceState::present);

            commandList_->close();

            renderSystem_->executeCommandList(commandList_);
        }
    }

    void SceneManager::presentBackBuffer()
    {
        renderSystem_->presentBackBuffer();
    }
}
