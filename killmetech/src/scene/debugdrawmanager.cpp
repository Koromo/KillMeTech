#include "debugdrawmanager.h"
#include "scene.h"
#include "camera.h"
#include "../renderer/rendersystem.h"
#include "../renderer/vertexdata.h"
#include "../renderer/commandlist.h"
#include "../renderer/constantbuffer.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/rootsignature.h"
#include "../renderer/shader.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../renderer/pipelinestate.h"
#include "../resources/resourcemanager.h"
#include "../core/math/matrix44.h"
#include "../core/math/vector3.h"
#include "../core/math/color.h"
#include "../core/string.h"
#include <Windows.h>

namespace killme
{
    DebugDrawManager debugDrawManager;

#ifdef KILLME_DEBUG

    void DebugDrawManager::startup(const std::shared_ptr<RenderSystem>& renderSystem)
    {
        renderSystem_ = renderSystem;

        // Create render resources
        viewProjBuffer_ = renderSystem_->createConstantBuffer(sizeof(Matrix44) * 2);
        viewProjHeap_ = renderSystem_->createGpuResourceHeap(1, GpuResourceHeapType::cbv_srv, GpuResourceHeapFlag::shaderVisible);
        renderSystem_->createGpuResourceView(viewProjHeap_, 0, viewProjBuffer_);

        RootSignatureDescription rootSigDesc(1);
        rootSigDesc[0].asTable(1, ShaderType::vertex);
        rootSigDesc[0][0].as(GpuResourceRangeType::cbv, 0, 1);

        const auto rootSig = renderSystem_->createRootSignature(rootSigDesc);

        const auto vs = accessResource<VertexShader>([]()
        {
            return compileShader<VertexShader>(toCharSet("media/debugdraw_vs.vhlsl"));
        });
        const auto ps = accessResource<PixelShader>([]()
        {
            return compileShader<PixelShader>(toCharSet("media/debugdraw_ps.phlsl"));
        });

        PipelineStateDescription pipelineDesc;
        pipelineDesc.rootSignature = rootSig;
        pipelineDesc.vertexShader = vs;
        pipelineDesc.pixelShader = ps;
        pipelineDesc.blend = BlendState::DEFAULT;
        pipeline_ = renderSystem_->createPipelineState(pipelineDesc);

        commandList_ = renderSystem_->createCommandList();

        RECT clientRect;
        GetClientRect(renderSystem_->getTargetWindow(), &clientRect);
        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        scissorRect_.top = 0;
        scissorRect_.left = 0;
        scissorRect_.right = static_cast<int>(clientWidth);
        scissorRect_.bottom = static_cast<int>(clientHeight);

        clear();
    }

    void DebugDrawManager::shutdown()
    {
        commandList_.reset();
        pipeline_.reset();
        viewProjHeap_.reset();
        viewProjBuffer_.reset();
        renderSystem_.reset();
        clear();
    }

    void DebugDrawManager::line(const Vector3& from, const Vector3& to, const Color& color)
    {
        positions_.emplace_back(from);
        positions_.emplace_back(to);
        colors_.emplace_back(color);
        colors_.emplace_back(color);
    }

    void DebugDrawManager::clear()
    {
        positions_.clear();
        colors_.clear();
    }

    void DebugDrawManager::debugDraw(const Camera& camera, const FrameResource& frame)
    {
        const auto numVertices = positions_.size();
        if (numVertices == 0)
        {
            return;
        }

        // Create render resources that includes all debugs
        const auto viewport = camera.getViewport();

        const auto viewMat = transpose(camera.getViewMatrix());
        const auto projMat = transpose(camera.getProjectionMatrix());

        viewProjBuffer_->update(&viewMat, 0, sizeof(Matrix44));
        viewProjBuffer_->update(&projMat, sizeof(Matrix44), sizeof(Matrix44));

        const auto positionBuffer = renderSystem_->createVertexBuffer(
            positions_.data(), numVertices * sizeof(float) * 3, sizeof(float) * 3);
        const auto colorBuffer = renderSystem_->createVertexBuffer(
            colors_.data(), numVertices * sizeof(float) * 4, sizeof(float) * 4);

        VertexData vertexData;
        vertexData.addVertices(SemanticNames::position, 0, positionBuffer);
        vertexData.addVertices(SemanticNames::color, 0, colorBuffer);

        const auto rootSignature = pipeline_->describe().rootSignature;
        const auto inputLayout = pipeline_->describe().vertexShader.access()->getInputLayout();
        const auto& binder = vertexData.getBinder(inputLayout);

        // Begin drawing to all debugs
        renderSystem_->beginCommands(commandList_, pipeline_);

        commandList_->resourceBarrior(frame.backBuffer, ResourceState::present, ResourceState::renderTarget);
        commandList_->setRenderTarget(frame.backBufferView, frame.depthStencilView);
        commandList_->setViewport(viewport);
        commandList_->setScissorRect(scissorRect_);
        commandList_->setPrimitiveTopology(PrimitiveTopology::lineList);
        commandList_->setVertexBuffers(binder);
        commandList_->setRootSignature(rootSignature);
        const auto heaps = { viewProjHeap_ };
        commandList_->setGpuResourceHeaps(heaps);
        commandList_->setGpuResourceTable(0, viewProjHeap_);
        commandList_->draw(numVertices);
        commandList_->resourceBarrior(frame.backBuffer, ResourceState::renderTarget, ResourceState::present);
        commandList_->endCommands();

        renderSystem_->executeCommands(commandList_);

        clear();
    }

#else

    void DebugDrawManager::startup(const std::shared_ptr<RenderSystem>&) {}
    void DebugDrawManager::shutdown() {}
    void DebugDrawManager::line(const Vector3&, const Vector3&, const Color&) {}
    void DebugDrawManager::clear() {}
    void DebugDrawManager::debugDraw(const Camera&, const FrameResource&) {}

#endif
}