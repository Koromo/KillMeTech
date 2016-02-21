#include "effectpass.h"
#include "effectshaderref.h"
#include "../renderer/rootsignature.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/shader.h"
#include "../renderer/constantbuffer.h"
#include "../renderer/rendersystem.h"

namespace killme
{
    EffectPass::EffectPass(RenderSystem& renderSystem,
        const std::shared_ptr<EffectShaderRef>& vsRef, const std::shared_ptr<EffectShaderRef>& psRef)
        : pipeline_()
        , resourceHeap_()
        , resourceHeapTable_()
        , vsRef_(vsRef)
        , psRef_(psRef)
    {
        const auto vsCBuffer = vsRef_->describeConstantBuffer();
        const auto psCBuffer = psRef_->describeConstantBuffer();

        auto numBuffers = 0;
        numBuffers += vsCBuffer.first ? 1 : 0;
        numBuffers += psCBuffer.first ? 1 : 0;

        if (numBuffers > 0)
        {
            resourceHeap_ = renderSystem.createGpuResourceHeap(numBuffers, GpuResourceHeapType::constantBuffer, GpuResourceHeapFlag::shaderVisible);
        }

        RootSignatureDescription rootSigDesc(numBuffers);
        size_t rootParamIndex = 0;
        size_t offsetInHeap = 0;

        if (vsCBuffer.first)
        {
            rootSigDesc[rootParamIndex].initialize(1, ShaderType::vertex);
            rootSigDesc[rootParamIndex][0].set(vsCBuffer.first->getRegisterSlot(), 1, offsetInHeap);
            renderSystem.createGpuResourceView(resourceHeap_, offsetInHeap, vsCBuffer.second);
            resourceHeapTable_.emplace(rootParamIndex, resourceHeap_);
            ++rootParamIndex;
            ++offsetInHeap;
        }
        if (psCBuffer.first)
        {
            rootSigDesc[rootParamIndex].initialize(1, ShaderType::pixel);
            rootSigDesc[rootParamIndex][0].set(psCBuffer.first->getRegisterSlot(), 1, offsetInHeap);
            renderSystem.createGpuResourceView(resourceHeap_, offsetInHeap, psCBuffer.second);
            resourceHeapTable_.emplace(rootParamIndex, resourceHeap_);
        }

        PipelineStateDescription pipelineDesc;
        pipelineDesc.rootSignature = renderSystem.createRootSignature(rootSigDesc);
        pipelineDesc.vertexShader = vsRef_->getReferenceVertexShader();
        pipelineDesc.pixelShader = psRef_->getReferencePixelShader();
        pipeline_ = renderSystem.createPipelineState(pipelineDesc);
    }

    void EffectPass::updateConstant(const std::string& param, const void* data)
    {
        vsRef_->updateConstant(param, data);
        psRef_->updateConstant(param, data);
    }

    std::shared_ptr<PipelineState> EffectPass::getPipelineState()
    {
        return pipeline_;
    }
}