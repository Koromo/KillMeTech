#include "material.h"
#include "scenemanager.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../renderer/rootsignature.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/rendersystem.h"
#include <algorithm>

namespace killme
{
    Material::Material(const Resource<VertexShader>& vs, const Resource<PixelShader>& ps)
        : pipelineState_()
        , cbufferHeap_()
        , indexMap_()
        , vsParamDesc_()
        , psParamDesc_()
        , vsParamBuffer_()
        , psParamBuffer_()
    {
        // Create the root signature
        const auto vsCbuffers = vs.access()->describeConstnatBuffers();
        const auto psCbuffers = ps.access()->describeConstnatBuffers();

        RootSignatureDescription rootSigDesc(2);
        rootSigDesc[0].initialize(vsCbuffers.size(), ShaderType::vertex);
        rootSigDesc[1].initialize(psCbuffers.size(), ShaderType::pixel);

        // Define ranges
        int indexInHeap = 0;
        int indexOfRange = 0;
        for (const auto& cb : vsCbuffers)
        {
            rootSigDesc[0][indexOfRange].set(cb.getRegisterSlot(), 1, indexInHeap);
            indexMap_[cb.getName()] = indexInHeap;

            if (cb.getName() == "VSParam")
            {
                // Store the description that named by VSParam
                vsParamDesc_ = cb;
            }
            ++indexInHeap;
            ++indexOfRange;
        }

        indexOfRange = 0;
        for (const auto& cb : psCbuffers)
        {
            rootSigDesc[1][indexOfRange].set(cb.getRegisterSlot(), 1, indexInHeap);
            indexMap_[cb.getName()] = indexInHeap;

            if (cb.getName() == "PSParam")
            {
                // Store the description that named by PSParam
                psParamDesc_ = cb;
            }
            ++indexInHeap;
            ++indexOfRange;
        }

        const auto rootSignature = renderSystem.createRootSignature(rootSigDesc);

        // Create the pipeline state
        PipelineStateDescription psDesc;
        psDesc.rootSignature = rootSignature;
        psDesc.vertexShader = vs;
        psDesc.pixelShader = ps;
        pipelineState_ = renderSystem.createPipelineState(psDesc);

        // Create the constant buffer heap
        cbufferHeap_ = renderSystem.createGpuResourceHeap(indexInHeap, GpuResourceHeapType::constantBuffer, GpuResourceHeapFlag::shaderVisible);

        // Set scene resource heaps
        sceneManager.setSceneResourceHeaps(*this);

        // Create the constant buffer for VSParam and PSParam
        if (vsParamDesc_)
        {
            vsParamBuffer_ = renderSystem.createConstantBuffer(vsParamDesc_->getSize());
            storeConstantBuffer(vsParamDesc_->getName(), vsParamBuffer_);
            for (const auto var : vsParamDesc_->describeVariables())
            {
                if (var.second.defValue)
                {
                    setVariableImpl(var.first, var.second.defValue.get());
                }
            }
        }
        if (psParamDesc_)
        {
            psParamBuffer_ = renderSystem.createConstantBuffer(psParamDesc_->getSize());
            storeConstantBuffer(psParamDesc_->getName(), psParamBuffer_);
            for (const auto var : psParamDesc_->describeVariables())
            {
                if (var.second.defValue)
                {
                    setVariableImpl(var.first, var.second.defValue.get());
                }
            }
        }
    }

    void Material::storeConstantBuffer(const std::string& name, const std::shared_ptr<ConstantBuffer>& buffer)
    {
        const auto it = indexMap_.find(name);
        if (it != std::cend(indexMap_))
        {
            renderSystem.storeGpuResource(cbufferHeap_, it->second, buffer);
        }
    }

    std::shared_ptr<PipelineState> Material::getPipelineState()
    {
        return pipelineState_;
    }

    std::shared_ptr<GpuResourceHeap> Material::getConstantBufferHeap()
    {
        return cbufferHeap_;
    }

    void Material::setVariableImpl(const std::string& name, const void* data)
    {
        if (const auto var = vsParamDesc_ ? vsParamDesc_->describeVariable(name) : nullopt)
        {
            const auto size = var->size;
            const auto offset = var->offset;
            vsParamBuffer_->update(data, offset, size);
        }
        else if (const auto var = psParamDesc_ ? psParamDesc_->describeVariable(name) : nullopt)
        {
            const auto size = var->size;
            const auto offset = var->offset;
            psParamBuffer_->update(data, offset, size);
        }
    }
}