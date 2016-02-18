#include "material.h"
#include "scene.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../renderer/rootsignature.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/rendersystem.h"
#include <assert.h>

namespace killme
{
    Material::Material(RenderSystem& renderSystem, const Resource<VertexShader>& vs, const Resource<PixelShader>& ps)
        : pipeline_()
        , paramHeap_()
        , paramHeapTable_()
        , vsParamDesc_()
        , psParamDesc_()
        , vsParamBuffer_()
        , psParamBuffer_()
    {
        // Create the root signature
        const auto vsCbuffers = vs.access()->describeConstnatBuffers();
        const auto psCbuffers = ps.access()->describeConstnatBuffers();

        assert(vsCbuffers.length() <= 1 && "Invalid vertex shader.");
        assert(psCbuffers.length() <= 1 && "Invalid pixel shader.");

        const auto numBuffers = vsCbuffers.length() + psCbuffers.length();
        RootSignatureDescription rootSigDesc(numBuffers);
        paramHeap_ = renderSystem.createGpuResourceHeap(numBuffers, GpuResourceHeapType::constantBuffer, GpuResourceHeapFlag::shaderVisible);

        size_t rootParamIndex = 0;
        size_t offsetInHeap = 0;

        if (vsCbuffers.length() == 1)
        {
            vsParamDesc_ = *std::cbegin(vsCbuffers);
            vsParamBuffer_ = initParams(renderSystem, *vsParamDesc_, rootSigDesc, ShaderType::vertex, rootParamIndex, offsetInHeap);
            ++rootParamIndex;
            ++offsetInHeap;
        }
        if (psCbuffers.length() == 1)
        {
            psParamDesc_ = *std::cbegin(psCbuffers);
            psParamBuffer_ = initParams(renderSystem, *psParamDesc_, rootSigDesc, ShaderType::pixel, rootParamIndex, offsetInHeap);
        }

        const auto rootSignature = renderSystem.createRootSignature(rootSigDesc);

        PipelineStateDescription pipelineDesc;
        pipelineDesc.rootSignature = rootSignature;
        pipelineDesc.vertexShader = vs;
        pipelineDesc.pixelShader = ps;
        pipeline_ = renderSystem.createPipelineState(pipelineDesc);
    }

    std::shared_ptr<PipelineState> Material::getPipelineState()
    {
        return pipeline_;
    }

    std::shared_ptr<ConstantBuffer> Material::initParams(RenderSystem& renderSystem,
        const ConstantBufferDescription& paramDesc, RootSignatureDescription& rootSigDesc,
        ShaderType visible, size_t rootParamIndex, size_t offsetInHeap)
    {
        rootSigDesc[rootParamIndex].initialize(1, visible);
        rootSigDesc[rootParamIndex][0].set(paramDesc.getRegisterSlot(), 1, offsetInHeap);

        const auto buffer = renderSystem.createConstantBuffer(paramDesc.getSize());
        renderSystem.createGpuResourceView(paramHeap_, offsetInHeap, buffer);
        paramHeapTable_.emplace(rootParamIndex, paramHeap_);

        for (const auto var : paramDesc.describeVariables())
        {
            if (var.second.defaultValue)
            {
                const auto p = var.second.defaultValue.get();
                buffer->update(p, var.second.offset, var.second.size);
            }
        }
        
        return buffer;
    }

    void Material::setVariableImpl(const std::string& name, const void* data)
    {
        const auto b = !!vsParamDesc_->describeVariable(name);
        if (const auto var = (vsParamDesc_ ? vsParamDesc_->describeVariable(name) : nullopt))
        {
            const auto size = var->size;
            const auto offset = var->offset;
            vsParamBuffer_->update(data, offset, size);
        }
        else if (const auto var = (psParamDesc_ ? psParamDesc_->describeVariable(name) : nullopt))
        {
            const auto size = var->size;
            const auto offset = var->offset;
            psParamBuffer_->update(data, offset, size);
        }
    }
}