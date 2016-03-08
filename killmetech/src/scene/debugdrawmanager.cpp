#include "debugdrawmanager.h"
#include "scene.h"
#include "camera.h"
#include "../renderer/rendersystem.h"
#include "../renderer/vertexdata.h"
#include "../renderer/commandlist.h"
#include "../renderer/constantbuffer.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/rootsignature.h"
#include "../renderer/shaders.h"
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

    void DebugDrawManager::initialize(const std::shared_ptr<RenderSystem>& renderSystem)
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

        const auto vs = Resource<VertexShader>([] { return compileHlslShader<VertexShader>(toCharSet("media/debugdraw_vs.vhlsl")); });
        const auto ps = Resource<PixelShader>([] { return compileHlslShader<PixelShader>(toCharSet("media/debugdraw_ps.phlsl")); });

        PipelineStateDescription pipelineDesc;
        pipelineDesc.rootSignature = rootSig;
        pipelineDesc.vertexShader = vs;
        pipelineDesc.pixelShader = ps;
        pipelineDesc.blend = BlendState::DEFAULT;
        pipeline_ = renderSystem_->createPipelineState(pipelineDesc);

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

    void DebugDrawManager::finalize()
    {
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
        const auto& views = vertexData.getVertexViews(inputLayout);

        // Begin drawing to all debugs
        const auto commands = renderSystem_->beginCommands(pipeline_);

        commands->resourceBarrior(frame.backBuffer, ResourceState::present, ResourceState::renderTarget);
        commands->setRenderTarget(frame.backBufferView, frame.depthStencilView);
        commands->setViewport(viewport);
        commands->setScissorRect(scissorRect_);
        commands->setPrimitiveTopology(PrimitiveTopology::lineList);
        commands->setVertexBuffers(views);
        commands->setRootSignature(rootSignature);
        const auto heaps = { viewProjHeap_ };
        commands->setGpuResourceHeaps(heaps);
        commands->setGpuResourceTable(0, viewProjHeap_);
        commands->draw(numVertices);
        commands->resourceBarrior(frame.backBuffer, ResourceState::renderTarget, ResourceState::present);
        commands->close();

        renderSystem_->executeCommands(commands);

        clear();
    }
}