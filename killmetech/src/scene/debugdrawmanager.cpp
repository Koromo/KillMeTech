#include "debugdrawmanager.h"
#include "scenemanager.h"
#include "scenevisitor.h"
#include "scenenode.h"
#include "camera.h"
#include "../renderer/rendersystem.h"
#include "../renderer/vertexdata.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/shader.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../renderer/commandlist.h"
#include "../renderer/rendertarget.h"
#include "../renderer/rootsignature.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/constantbuffer.h"
#include "../core/string.h"
#include "../core/math/vector3.h"
#include "../core/math/color.h"
#include "../core/math/matrix44.h"
#include "../resource/resourcemanager.h"

namespace killme
{
    DebugDrawManager debugDrawManager;

#ifdef KILLME_DEBUG

    void DebugDrawManager::startup()
    {
        // Create render resources
        viewProjBuffer_ = renderSystem.createConstantBuffer(sizeof(Matrix44) * 2);
        viewProjHeap_ = renderSystem.createGpuResourceHeap(1, GpuResourceHeapType::constantBuffer, GpuResourceHeapFlag::shaderVisible);
        renderSystem.storeGpuResource(viewProjHeap_, 0, viewProjBuffer_);

        RootSignatureDescription rootSigDesc(1);
        rootSigDesc[0].initialize(1, ShaderType::vertex);
        rootSigDesc[0][0].set(0, 1);

        const auto rootSig = renderSystem.createRootSignature(rootSigDesc);
        const auto vs = getResourceInterface<VertexShader>("media/debugdraw_vs.vhlsl");
        const auto ps = getResourceInterface<PixelShader>("media/debugdraw_ps.phlsl");

        PipelineStateDescription pipelineDesc;
        pipelineDesc.rootSignature = rootSig;
        pipelineDesc.vertexShader = vs;
        pipelineDesc.pixelShader = ps;
        pipeline_ = renderSystem.createPipelineState(pipelineDesc);

        commandList_ = renderSystem.createCommandList();

        RECT clientRect;
        GetClientRect(renderSystem.getTargetWindow(), &clientRect);
        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        scissorRect_.top = 0;
        scissorRect_.left = 0;
        scissorRect_.right = static_cast<int>(clientWidth);
        scissorRect_.bottom = static_cast<int>(clientHeight);
    }

    void DebugDrawManager::shutdown()
    {
        commandList_.reset();
        pipeline_.reset();
        viewProjHeap_.reset();
        viewProjBuffer_.reset();
        colors_.clear();
        positions_.clear();
    }

    void DebugDrawManager::line(const Vector3& from, const Vector3& to, const Color& color)
    {
        positions_.push_back(from);
        positions_.push_back(to);
        colors_.push_back(color);
        colors_.push_back(color);
    }

    namespace
    {
        struct FindCamera : SceneVisitor
        {
            std::shared_ptr<Camera> camera;
            bool operator()(const std::shared_ptr<SceneNode>& n, const std::shared_ptr<Camera>& c)
            {
                camera = c;
                return true;
            }
        };
    }

    void DebugDrawManager::drawDebugs()
    {
        const auto numVertices = positions_.size();
        if (numVertices == 0)
        {
            return;
        }

        // Find a camera
        FindCamera visitor;
        sceneManager.getRootNode()->depthTraverse(visitor);

        const auto camera = visitor.camera;
        if (!camera)
        {
            positions_.clear();
            colors_.clear();
            return;
        }

        // Create render resources that includes all debugs
        const auto viewport = camera->getViewport();

        const auto viewMat = transpose(inverse(camera->lockOwner()->getWorldMatrix()));
        const auto projMat = transpose(camera->getProjectionMatrix());
        viewProjBuffer_->update(&viewMat, 0, sizeof(Matrix44));
        viewProjBuffer_->update(&projMat, sizeof(Matrix44), sizeof(Matrix44));

        const auto positionBuffer = renderSystem.createVertexBuffer(
            positions_.data(), numVertices * sizeof(float) * 3, sizeof(float) * 3);
        const auto colorBuffer = renderSystem.createVertexBuffer(
            colors_.data(), numVertices * sizeof(float) * 4, sizeof(float) * 4);

        VertexData vertexData;
        vertexData.addVertices(VertexSemantic::position, 0, positionBuffer);
        vertexData.addVertices(VertexSemantic::color, 0, colorBuffer);

        const auto rootSignature = pipeline_->describe().rootSignature;
        const auto inputLayout = pipeline_->describe().vertexShader.access()->getInputLayout();
        const auto& binder = vertexData.getBinder(inputLayout);

        // Begin drawing to all debugs
        const auto renderTarget = renderSystem.getCurrentBackBuffer();
        const auto depthStencil = renderSystem.getDepthStencil();

        renderSystem.resetCommandList(commandList_, pipeline_);

        commandList_->resourceBarrior(renderTarget, ResourceState::present, ResourceState::renderTarget);
        commandList_->setRenderTarget(renderTarget, depthStencil);
        commandList_->setViewport(viewport);
        commandList_->setScissorRect(scissorRect_);
        commandList_->setPrimitiveTopology(PrimitiveTopology::lineList);
        commandList_->setVertexBuffers(binder);
        commandList_->setRootSignature(rootSignature);
        const auto heaps = { viewProjHeap_ };
        commandList_->setGpuResourceHeaps(heaps, 1);
        commandList_->setGpuResourceTable(0, viewProjHeap_);
        commandList_->draw(numVertices);
        commandList_->resourceBarrior(renderTarget, ResourceState::renderTarget, ResourceState::present);
        commandList_->close();

        renderSystem.executeCommandList(commandList_);

        positions_.clear();
        colors_.clear();
    }

#else

    void DebugDrawManager::startup() {}
    void DebugDrawManager::shutdown() {}
    void DebugDrawManager::line(const Vector3& from, const Vector3& to, const Color& color) {}
    void DebugDrawManager::drawDebugs() {}

#endif
}