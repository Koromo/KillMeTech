#include "effectpass.h"
#include "effectshaderref.h"
#include "../renderer/rootsignature.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/shader.h"
#include "../renderer/constantbuffer.h"
#include "../renderer/rendersystem.h"
#include "../renderer/texture.h"
#include <utility>

namespace killme
{
    namespace
    {
        struct RangeInfo
        {
            GpuResourceRangeType type;
            size_t baseRegister;
            size_t numResources; // 1
            size_t offsetInHeap;
        };

        struct RootParamInfo
        {
            std::shared_ptr<EffectShaderRef> boundShader;
            std::unordered_map<std::string, size_t> resourceToRange;
            size_t numRanges; // Same to the count of all resources in this root param
            ShaderType visible;
            std::vector<RangeInfo> rangeInfos;
        };

        template <class Shaders>
        std::vector<RootParamInfo> getRootParamInfosAbout_cbv_srv(const Shaders& shaders)
        {
            std::vector<RootParamInfo> rootParamInfos;
            auto offsetInHeap = 0;
            for (const auto& shader : shaders)
            {
                const auto& cbufferDesc = shader->describeConstantBuffer();
                const auto& textureDescs = shader->describeBoundResources(BoundResourceType::texture);
                if (!cbufferDesc && textureDescs.length() == 0)
                {
                    continue;
                }

                RootParamInfo rootParamInfo;
                rootParamInfo.boundShader = shader;
                rootParamInfo.visible = shader->getType();
                rootParamInfo.numRanges = (cbufferDesc ? 1 : 0) + textureDescs.length();

                if (cbufferDesc)
                {
                    RangeInfo rangeInfo;
                    rangeInfo.type = GpuResourceRangeType::cbv;
                    rangeInfo.baseRegister = cbufferDesc->getRegisterSlot();
                    rangeInfo.numResources = 1;
                    rangeInfo.offsetInHeap = offsetInHeap;
                    rootParamInfo.resourceToRange.emplace(cbufferDesc->getName(), rootParamInfo.rangeInfos.size());
                    rootParamInfo.rangeInfos.emplace_back(rangeInfo);
                    ++offsetInHeap;
                }

                for (const auto& texDesc : textureDescs)
                {
                    RangeInfo rangeInfo;
                    rangeInfo.type = GpuResourceRangeType::srv;
                    rangeInfo.baseRegister = texDesc.getRegisterSlot();
                    rangeInfo.numResources = 1;
                    rangeInfo.offsetInHeap = offsetInHeap;
                    rootParamInfo.resourceToRange.emplace(texDesc.getName(), rootParamInfo.rangeInfos.size());
                    rootParamInfo.rangeInfos.emplace_back(rangeInfo);
                    ++offsetInHeap;
                }

                rootParamInfos.emplace_back(rootParamInfo);
            }

            return rootParamInfos;
        }

        template <class Shaders>
        std::vector<RootParamInfo> getRootParamInfosAbout_sampler(const Shaders& shaders)
        {
            std::vector<RootParamInfo> rootParamInfos;
            auto offsetInHeap = 0;
            for (const auto& shader : shaders)
            {
                const auto& samplerDescs = shader->describeBoundResources(BoundResourceType::sampler);
                if (samplerDescs.length() == 0)
                {
                    continue;
                }

                RootParamInfo rootParamInfo;
                rootParamInfo.boundShader = shader;
                rootParamInfo.visible = shader->getType();
                rootParamInfo.numRanges = samplerDescs.length();

                for (const auto& samDesc : samplerDescs)
                {
                    RangeInfo rangeInfo;
                    rangeInfo.type = GpuResourceRangeType::sampler;
                    rangeInfo.baseRegister = samDesc.getRegisterSlot();
                    rangeInfo.numResources = 1;
                    rangeInfo.offsetInHeap = offsetInHeap;
                    rootParamInfo.resourceToRange.emplace(samDesc.getName(), rootParamInfo.rangeInfos.size());
                    rootParamInfo.rangeInfos.emplace_back(rangeInfo);
                    ++offsetInHeap;
                }

                rootParamInfos.emplace_back(rootParamInfo);
            }

            return rootParamInfos;
        }

        size_t getNumResourcesInHeap(const std::vector<RootParamInfo>& rootParams)
        {
            auto sum = 0;
            for (const auto& info : rootParams)
            {
                sum += info.numRanges;
            }
            return sum;
        }
    }

    /// TODO:
    EffectPass::EffectPass(const std::shared_ptr<RenderSystem>& renderSystem, const EffectPassCreation& creation)
        : renderSystem_(renderSystem)
        , pipeline_()
        , heap_cbv_srv_()
        , heap_sampler_()
        , resourceHeapTables_()
        , constantUpdateInfoMap_()
        , textureUpdateInfoMap_()
        , vsRef_(creation.vsRef)
        , psRef_(creation.psRef)
        , forEachLight_(creation.forEachLight)
    {
        assert((vsRef_ && psRef_) && "Effect pass creation error.");

        const auto shaders = { vsRef_, psRef_ };

        const auto rootParams_cbv_srv = getRootParamInfosAbout_cbv_srv(shaders);
        const auto rootParams_sampler = getRootParamInfosAbout_sampler(shaders);

        const auto numResources_cbv_srv = getNumResourcesInHeap(rootParams_cbv_srv);
        const auto numResources_sampler = getNumResourcesInHeap(rootParams_sampler);

        if (numResources_cbv_srv > 0)
        {
            heap_cbv_srv_ = renderSystem_->createGpuResourceHeap(numResources_cbv_srv, GpuResourceHeapType::cbv_srv, GpuResourceHeapFlag::shaderVisible);
        }
        if (numResources_sampler > 0)
        {
            heap_sampler_ = renderSystem_->createGpuResourceHeap(numResources_sampler, GpuResourceHeapType::sampler, GpuResourceHeapFlag::shaderVisible);
        }

        const auto numRootParams_cbv_srv = rootParams_cbv_srv.size();
        const auto numRootParams = numRootParams_cbv_srv + rootParams_sampler.size();

        std::vector<RootParamInfo> rootParamInfos(std::move(rootParams_cbv_srv));
        rootParamInfos.reserve(numRootParams);
        rootParamInfos.insert(std::cend(rootParamInfos), std::cbegin(rootParams_sampler), std::cend(rootParams_sampler));

        RootSignatureDescription rootSigDesc(numRootParams);
        for (size_t rootParamIndex = 0; rootParamIndex < numRootParams_cbv_srv; ++rootParamIndex)
        {
            const auto& rootParamInfo = rootParamInfos[rootParamIndex];
            rootSigDesc[rootParamIndex].asTable(rootParamInfo.numRanges, rootParamInfo.visible);
            resourceHeapTables_.emplace(rootParamIndex, heap_cbv_srv_);

            for (size_t rangeIndex = 0; rangeIndex < rootParamInfo.numRanges; ++rangeIndex)
            {
                const auto& rangeInfo = rootParamInfo.rangeInfos[rangeIndex];

                // Initialize as the constant buffer
                if (rangeInfo.type == GpuResourceRangeType::cbv)
                {
                    const auto& cbufferDesc = rootParamInfo.boundShader->describeConstantBuffer();
                    const auto cbuffer = renderSystem_->createConstantBuffer(cbufferDesc->getSize());
                    for (const auto var : cbufferDesc->describeVariables())
                    {
                        if (var.second.defaultValue)
                        {
                            const auto p = var.second.defaultValue.get();
                            cbuffer->update(p, var.second.offset, var.second.size);
                        }
                    }

                    const auto& constantMap = rootParamInfo.boundShader->getConstantMap();
                    for (const auto& pair : constantMap)
                    {
                        detail::ConstantUpdateInfo updateInfo;
                        updateInfo.desc = *cbufferDesc->describeVariable(pair.second);
                        updateInfo.dest = cbuffer;
                        constantUpdateInfoMap_.emplace(pair.first, updateInfo);
                    }

                    renderSystem_->createGpuResourceView(heap_cbv_srv_, rangeInfo.offsetInHeap, cbuffer);
                    rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::cbv,
                        rangeInfo.baseRegister, rangeInfo.numResources, rangeInfo.offsetInHeap);
                }

                // Initialize as the texture
                else if (rangeInfo.type == GpuResourceRangeType::srv)
                {
                    rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::srv,
                        rangeInfo.baseRegister, rangeInfo.numResources, rangeInfo.offsetInHeap);
                }
            }

            const auto& textureMap = rootParamInfo.boundShader->getTextureMap();
            for (const auto& pair : textureMap)
            {
                const auto& rangeInfo = rootParamInfo.rangeInfos[rootParamInfo.resourceToRange.at(pair.second)];
                detail::TextureUpdateInfo updateInfo;
                updateInfo.tex_index = rangeInfo.offsetInHeap;
                textureUpdateInfoMap_.emplace(pair.first, updateInfo);
            }
        }

        for (size_t rootParamIndex = numRootParams_cbv_srv; rootParamIndex < numRootParams; ++rootParamIndex)
        {
            const auto& rootParamInfo = rootParamInfos[rootParamIndex];
            rootSigDesc[rootParamIndex].asTable(rootParamInfo.numRanges, rootParamInfo.visible);
            resourceHeapTables_.emplace(rootParamIndex, heap_sampler_);

            for (size_t rangeIndex = 0; rangeIndex < rootParamInfo.numRanges; ++rangeIndex)
            {
                const auto& rangeInfo = rootParamInfo.rangeInfos[rangeIndex];
                assert(rangeInfo.type == GpuResourceRangeType::sampler && "Logic error.");
                rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::sampler,
                    rangeInfo.baseRegister, rangeInfo.numResources, rangeInfo.offsetInHeap);
            }

            const auto& samplerMap = rootParamInfo.boundShader->getSamplerMap();
            for (const auto& pair : samplerMap)
            {
                const auto& rangeInfo = rootParamInfo.rangeInfos[rootParamInfo.resourceToRange.at(pair.second)];
                auto& updateInfo = textureUpdateInfoMap_.at(pair.first);
                updateInfo.sam_index = rangeInfo.offsetInHeap;
                updateInfo.sam_current = std::make_shared<Sampler>();
                renderSystem_->createGpuResourceView(heap_sampler_, updateInfo.sam_index, updateInfo.sam_current);
            }
        }

        PipelineStateDescription pipelineDesc;
        pipelineDesc.rootSignature = renderSystem_->createRootSignature(rootSigDesc);
        pipelineDesc.vertexShader = vsRef_->getReferenceVertexShader();
        pipelineDesc.pixelShader = psRef_->getReferencePixelShader();
        pipelineDesc.blend = creation.blendState;
        pipeline_ = renderSystem_->createPipelineState(pipelineDesc);
    }

    bool EffectPass::forEachLight() const
    {
        return forEachLight_;
    }

    void EffectPass::updateConstant(const std::string& param, const void* data)
    {
        const auto it = constantUpdateInfoMap_.find(param);
        if (it != std::cend(constantUpdateInfoMap_))
        {
            const auto size = it->second.desc.size;
            const auto offset = it->second.desc.offset;
            it->second.dest->update(data, offset, size);
        }
    }

    void EffectPass::updateTexture(const std::string& param, const Resource<Texture>& tex)
    {
        const auto it = textureUpdateInfoMap_.find(param);
        if (it != std::cend(textureUpdateInfoMap_))
        {
            renderSystem_->createGpuResourceView(heap_cbv_srv_, it->second.tex_index, tex.access());
            it->second.tex_current = tex;
        }
    }

    void EffectPass::updateSampler(const std::string& param, const std::shared_ptr<Sampler>& sam)
    {
        const auto it = textureUpdateInfoMap_.find(param);
        if (it != std::cend(textureUpdateInfoMap_))
        {
            renderSystem_->createGpuResourceView(heap_sampler_, it->second.sam_index, sam);
            it->second.sam_current = sam;
        }
    }

    std::shared_ptr<PipelineState> EffectPass::getPipelineState()
    {
        return pipeline_;
    }
}