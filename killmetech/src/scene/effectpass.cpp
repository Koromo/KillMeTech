#include "effectpass.h"
#include "materialcreation.h"
#include "../renderer/shader.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../renderer/texture.h"
#include "../renderer/rendersystem.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/rootsignature.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/constantbuffer.h"
#include "../resources/resource.h"
#include "../resources/resourcemanager.h"
#include <vector>
#include <utility>
#include <algorithm>

#undef min

namespace killme
{
    namespace
    {
        struct RangeInfo
        {
            GpuResourceRangeType type;
            std::string resourceName;
            size_t offsetInHeap;
            /// NOTE: numResource = 1
        };

        struct RootParamInfo
        {
            std::shared_ptr<BasicShader> shader;
            std::shared_ptr<GpuResourceHeap> heap;
            std::vector<RangeInfo> rangeInfos; /// NOTE: Count of range is same to the size of heap
        };

        template <class Shaders>
        std::vector<RootParamInfo> getRootParamInfos(const std::shared_ptr<RenderSystem>& renderSystem, Shaders shaders)
        {
            std::vector<RootParamInfo> rootParams;
            auto offsetInHeap_cbv_srv = 0;
            auto offsetInHeap_sampler = 0;

            for (const auto& shader : shaders)
            {
                // For cbv_srv
                RootParamInfo rootParam_cbv_srv;
                rootParam_cbv_srv.shader = shader;

                auto& resourceDescs = shader->describeBoundResources(BoundResourceType::cbuffer);
                for (const auto& desc : resourceDescs)
                {
                    RangeInfo range;
                    range.type = GpuResourceRangeType::cbv;
                    range.resourceName = desc.getName();
                    range.offsetInHeap = offsetInHeap_cbv_srv++;
                    rootParam_cbv_srv.rangeInfos.emplace_back(std::move(range));
                }

                resourceDescs = shader->describeBoundResources(BoundResourceType::texture);
                for (const auto& desc : resourceDescs)
                {
                    RangeInfo range;
                    range.type = GpuResourceRangeType::srv;
                    range.resourceName = desc.getName();
                    range.offsetInHeap = offsetInHeap_cbv_srv++;
                    rootParam_cbv_srv.rangeInfos.emplace_back(std::move(range));
                }

                if (!rootParam_cbv_srv.rangeInfos.empty())
                {
                    rootParams.emplace_back(std::move(rootParam_cbv_srv));
                }

                // For sampler
                RootParamInfo rootParam_sampler;
                rootParam_sampler.shader = shader;

                resourceDescs = shader->describeBoundResources(BoundResourceType::sampler);
                for (const auto& desc : resourceDescs)
                {
                    RangeInfo range;
                    range.type = GpuResourceRangeType::sampler;
                    range.resourceName = desc.getName();
                    range.offsetInHeap = offsetInHeap_sampler++;
                    rootParam_sampler.rangeInfos.emplace_back(std::move(range));
                }

                if (!rootParam_sampler.rangeInfos.empty())
                {
                    rootParams.emplace_back(std::move(rootParam_sampler));
                }
            }

            // Create resource heaps
            std::shared_ptr<GpuResourceHeap> heap_cbv_srv;
            if (offsetInHeap_cbv_srv > 0)
            {
                heap_cbv_srv = renderSystem->createGpuResourceHeap(offsetInHeap_cbv_srv, GpuResourceHeapType::cbv_srv, GpuResourceHeapFlag::shaderVisible);
            }

            std::shared_ptr<GpuResourceHeap> heap_sampler;
            if (offsetInHeap_sampler > 0)
            {
                heap_sampler = renderSystem->createGpuResourceHeap(offsetInHeap_sampler, GpuResourceHeapType::sampler, GpuResourceHeapFlag::shaderVisible);
            }

            for (auto& rootParam : rootParams)
            {
                if (rootParam.rangeInfos[0].type == GpuResourceRangeType::sampler)
                {
                    rootParam.heap = heap_sampler;
                }
                else
                {
                    rootParam.heap = heap_cbv_srv;
                }
            }

            return rootParams;
        }
    }

    EffectPass::EffectPass(const std::shared_ptr<RenderSystem>& renderSystem, ResourceManager& resourceManager,
        const MaterialDescription& matDesc, const PassDescription& passDesc)
        : renderSystem_(renderSystem)
        , pipeline_()
        , resourceHeaps_()
        , resourceHeapTables_()
        , constantUpdateInfoMap_()
        , textureUpdateInfoMap_()
        , samplerUpdateInfoMap_()
        , forEachLight_(passDesc.forEachLight)
    {
        const auto& vsBound = matDesc.getVShaderBound(passDesc.vsRef);
        const auto& psBound = matDesc.getPShaderBound(passDesc.psRef);
        const auto vertexShader = resourceManager.getAccessor<VertexShader>(vsBound.path, true);
        const auto pixelShader = resourceManager.getAccessor<PixelShader>(psBound.path, true);

        // Create root signature
        std::vector<std::shared_ptr<BasicShader>> shaders = { vertexShader.access(), pixelShader.access() };
        const auto rootParamInfos = getRootParamInfos(renderSystem, shaders);

        const auto numRootParams = rootParamInfos.size();
        RootSignatureDescription rootSigDesc(numRootParams);
        for (size_t rootParamIndex = 0; rootParamIndex < numRootParams; ++rootParamIndex) // For each params
        {
            // Create root parameter
            const auto& rootParamInfo = rootParamInfos[rootParamIndex];
            const auto numRanges = rootParamInfo.rangeInfos.size();

            rootSigDesc[rootParamIndex].asTable(numRanges, rootParamInfo.shader->getType());
            for (size_t rangeIndex = 0; rangeIndex < numRanges; ++rangeIndex) // For each ranges
            {
                // Create range
                const auto& rangeInfo = rootParamInfo.rangeInfos[rangeIndex];

                if (rangeInfo.type == GpuResourceRangeType::cbv) // Constant buffer
                {
                    // Initialize constant buffer
                    const auto& cbufferDesc = rootParamInfo.shader->describeConstantBuffer(rangeInfo.resourceName);
                    const auto cbuffer = renderSystem_->createConstantBuffer(cbufferDesc->getSize());
                    const auto& boundDesc = rootParamInfo.shader == vertexShader.access() ? vsBound : psBound;

                    for (const auto& var : cbufferDesc->describeVariables())
                    {
                        if (var.second.init)
                        {
                            cbuffer->update(var.second.init.get(), var.second.offset, var.second.size);
                        }

                        const auto it = boundDesc.constantMapping.find(var.first);
                        if (it != std::cend(boundDesc.constantMapping))
                        {
                            detail::ConstantUpdateInfo update;
                            update.desc = var.second;
                            update.dest = cbuffer;
                            constantUpdateInfoMap_.emplace(it->second, std::move(update));
                        }
                    }

                    // Initialize range and heap
                    renderSystem->createGpuResourceView(rootParamInfo.heap, rangeInfo.offsetInHeap, cbuffer);
                    rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::cbv, cbufferDesc->getRegisterSlot(), 1, rangeInfo.offsetInHeap);
                }
                else if (rangeInfo.type == GpuResourceRangeType::srv) // Texture
                {
                    const auto& boundDesc = rootParamInfo.shader == vertexShader.access() ? vsBound : psBound;
                    const auto it = boundDesc.textureMapping.find(rangeInfo.resourceName);
                    if (it != std::cend(boundDesc.textureMapping))
                    {
                        detail::TextureUpdateInfo update;
                        update.index = rangeInfo.offsetInHeap;
                        update.dest = rootParamInfo.heap;
                        textureUpdateInfoMap_.emplace(it->second, std::move(update));
                    }

                    const auto& resourceDesc = rootParamInfo.shader->describeBoundResource(rangeInfo.resourceName);
                    rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::srv, resourceDesc->getRegisterSlot(), 1, rangeInfo.offsetInHeap);
                }
                else if (rangeInfo.type == GpuResourceRangeType::sampler) // Sampler
                {
                    const auto& boundDesc = rootParamInfo.shader == vertexShader.access() ? vsBound : psBound;
                    const auto it = boundDesc.samplerMapping.find(rangeInfo.resourceName);
                    if (it != std::cend(boundDesc.samplerMapping))
                    {
                        detail::SamplerUpdateInfo update;
                        update.index = rangeInfo.offsetInHeap;
                        update.dest = rootParamInfo.heap;
                        samplerUpdateInfoMap_.emplace(it->second, std::move(update));
                    }

                    const auto& resourceDesc = rootParamInfo.shader->describeBoundResource(rangeInfo.resourceName);
                    rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::sampler, resourceDesc->getRegisterSlot(), 1, rangeInfo.offsetInHeap);
                }
            }

            // Store heap
            resourceHeaps_.emplace(rootParamInfo.heap);
            resourceHeapTables_.emplace(rootParamIndex, rootParamInfo.heap);
        }

        PipelineStateDescription pipelineDesc;
        pipelineDesc.rootSignature = renderSystem_->createRootSignature(rootSigDesc);
        pipelineDesc.vertexShader = vertexShader;
        pipelineDesc.pixelShader = pixelShader;
        pipelineDesc.blend = passDesc.blendState;
        pipeline_ = renderSystem_->createPipelineState(pipelineDesc);
    }

    bool EffectPass::forEachLight() const
    {
        return forEachLight_;
    }

    void EffectPass::updateConstant(const std::string& matParam, const void* data, size_t size)
    {
        const auto it = constantUpdateInfoMap_.find(matParam);
        if (it != std::cend(constantUpdateInfoMap_))
        {
            it->second.dest->update(data, it->second.desc.offset, std::min(size, it->second.desc.size));
        }
    }

    void EffectPass::updateTexture(const std::string& matParam, const Resource<Texture>& tex)
    {
        const auto it = textureUpdateInfoMap_.find(matParam);
        if (it != std::cend(textureUpdateInfoMap_))
        {
            renderSystem_->createGpuResourceView(it->second.dest, it->second.index, tex.access());
        }
    }

    void EffectPass::updateSampler(const std::string& matParam, const std::shared_ptr<Sampler>& sam)
    {
        const auto it = samplerUpdateInfoMap_.find(matParam);
        if (it != std::cend(samplerUpdateInfoMap_))
        {
            renderSystem_->createGpuResourceView(it->second.dest, it->second.index, sam);
        }
    }

    std::shared_ptr<PipelineState> EffectPass::getPipelineState()
    {
        return pipeline_;
    }
}

//#include "effectpass.h"
//#include "effectshaderref.h"
//#include "../renderer/rootsignature.h"
//#include "../renderer/pipelinestate.h"
//#include "../renderer/gpuresourceheap.h"
//#include "../renderer/shader.h"
//#include "../renderer/constantbuffer.h"
//#include "../renderer/rendersystem.h"
//#include "../renderer/texture.h"
//#include <utility>
//
//namespace killme
//{
//    namespace
//    {
//        struct RangeInfo
//        {
//            GpuResourceRangeType type;
//            size_t baseRegister;
//            size_t numResources; // 1
//            size_t offsetInHeap;
//        };
//
//        struct RootParamInfo
//        {
//            std::shared_ptr<EffectShaderRef> boundShader;
//            std::unordered_map<std::string, size_t> resourceToRange;
//            size_t numRanges; // Same to the count of all resources in this root param
//            ShaderType visible;
//            std::vector<RangeInfo> rangeInfos;
//        };
//
//        template <class Shaders>
//        std::vector<RootParamInfo> getRootParamInfosAbout_cbv_srv(const Shaders& shaders)
//        {
//            std::vector<RootParamInfo> rootParamInfos;
//            auto offsetInHeap = 0;
//            for (const auto& shader : shaders)
//            {
//                const auto& cbufferDesc = shader->describeConstantBuffer();
//                const auto& textureDescs = shader->describeBoundResources(BoundResourceType::texture);
//                if (!cbufferDesc && textureDescs.length() == 0)
//                {
//                    continue;
//                }
//
//                RootParamInfo rootParamInfo;
//                rootParamInfo.boundShader = shader;
//                rootParamInfo.visible = shader->getType();
//                rootParamInfo.numRanges = (cbufferDesc ? 1 : 0) + textureDescs.length();
//
//                if (cbufferDesc)
//                {
//                    RangeInfo rangeInfo;
//                    rangeInfo.type = GpuResourceRangeType::cbv;
//                    rangeInfo.baseRegister = cbufferDesc->getRegisterSlot();
//                    rangeInfo.numResources = 1;
//                    rangeInfo.offsetInHeap = offsetInHeap;
//                    rootParamInfo.resourceToRange.emplace(cbufferDesc->getName(), rootParamInfo.rangeInfos.size());
//                    rootParamInfo.rangeInfos.emplace_back(rangeInfo);
//                    ++offsetInHeap;
//                }
//
//                for (const auto& texDesc : textureDescs)
//                {
//                    RangeInfo rangeInfo;
//                    rangeInfo.type = GpuResourceRangeType::srv;
//                    rangeInfo.baseRegister = texDesc.getRegisterSlot();
//                    rangeInfo.numResources = 1;
//                    rangeInfo.offsetInHeap = offsetInHeap;
//                    rootParamInfo.resourceToRange.emplace(texDesc.getName(), rootParamInfo.rangeInfos.size());
//                    rootParamInfo.rangeInfos.emplace_back(rangeInfo);
//                    ++offsetInHeap;
//                }
//
//                rootParamInfos.emplace_back(rootParamInfo);
//            }
//
//            return rootParamInfos;
//        }
//
//        template <class Shaders>
//        std::vector<RootParamInfo> getRootParamInfosAbout_sampler(const Shaders& shaders)
//        {
//            std::vector<RootParamInfo> rootParamInfos;
//            auto offsetInHeap = 0;
//            for (const auto& shader : shaders)
//            {
//                const auto& samplerDescs = shader->describeBoundResources(BoundResourceType::sampler);
//                if (samplerDescs.length() == 0)
//                {
//                    continue;
//                }
//
//                RootParamInfo rootParamInfo;
//                rootParamInfo.boundShader = shader;
//                rootParamInfo.visible = shader->getType();
//                rootParamInfo.numRanges = samplerDescs.length();
//
//                for (const auto& samDesc : samplerDescs)
//                {
//                    RangeInfo rangeInfo;
//                    rangeInfo.type = GpuResourceRangeType::sampler;
//                    rangeInfo.baseRegister = samDesc.getRegisterSlot();
//                    rangeInfo.numResources = 1;
//                    rangeInfo.offsetInHeap = offsetInHeap;
//                    rootParamInfo.resourceToRange.emplace(samDesc.getName(), rootParamInfo.rangeInfos.size());
//                    rootParamInfo.rangeInfos.emplace_back(rangeInfo);
//                    ++offsetInHeap;
//                }
//
//                rootParamInfos.emplace_back(rootParamInfo);
//            }
//
//            return rootParamInfos;
//        }
//
//        size_t getNumResourcesInHeap(const std::vector<RootParamInfo>& rootParams)
//        {
//            auto sum = 0;
//            for (const auto& info : rootParams)
//            {
//                sum += info.numRanges;
//            }
//            return sum;
//        }
//    }
//
//    /// TODO:
//    EffectPass::EffectPass(const std::shared_ptr<RenderSystem>& renderSystem, const EffectPassCreation& creation)
//        : renderSystem_(renderSystem)
//        , pipeline_()
//        , heap_cbv_srv_()
//        , heap_sampler_()
//        , resourceHeapTables_()
//        , constantUpdateInfoMap_()
//        , textureUpdateInfoMap_()
//        , vsRef_(creation.vsRef)
//        , psRef_(creation.psRef)
//        , forEachLight_(creation.forEachLight)
//    {
//        assert((vsRef_ && psRef_) && "Effect pass creation error.");
//
//        const auto shaders = { vsRef_, psRef_ };
//
//        const auto rootParams_cbv_srv = getRootParamInfosAbout_cbv_srv(shaders);
//        const auto rootParams_sampler = getRootParamInfosAbout_sampler(shaders);
//
//        const auto numResources_cbv_srv = getNumResourcesInHeap(rootParams_cbv_srv);
//        const auto numResources_sampler = getNumResourcesInHeap(rootParams_sampler);
//
//        if (numResources_cbv_srv > 0)
//        {
//            heap_cbv_srv_ = renderSystem_->createGpuResourceHeap(numResources_cbv_srv, GpuResourceHeapType::cbv_srv, GpuResourceHeapFlag::shaderVisible);
//        }
//        if (numResources_sampler > 0)
//        {
//            heap_sampler_ = renderSystem_->createGpuResourceHeap(numResources_sampler, GpuResourceHeapType::sampler, GpuResourceHeapFlag::shaderVisible);
//        }
//
//        const auto numRootParams_cbv_srv = rootParams_cbv_srv.size();
//        const auto numRootParams = numRootParams_cbv_srv + rootParams_sampler.size();
//
//        std::vector<RootParamInfo> rootParamInfos(std::move(rootParams_cbv_srv));
//        rootParamInfos.reserve(numRootParams);
//        rootParamInfos.insert(std::cend(rootParamInfos), std::cbegin(rootParams_sampler), std::cend(rootParams_sampler));
//
//        RootSignatureDescription rootSigDesc(numRootParams);
//        for (size_t rootParamIndex = 0; rootParamIndex < numRootParams_cbv_srv; ++rootParamIndex)
//        {
//            const auto& rootParamInfo = rootParamInfos[rootParamIndex];
//            rootSigDesc[rootParamIndex].asTable(rootParamInfo.numRanges, rootParamInfo.visible);
//            resourceHeapTables_.emplace(rootParamIndex, heap_cbv_srv_);
//
//            for (size_t rangeIndex = 0; rangeIndex < rootParamInfo.numRanges; ++rangeIndex)
//            {
//                const auto& rangeInfo = rootParamInfo.rangeInfos[rangeIndex];
//
//                // Initialize as the constant buffer
//                if (rangeInfo.type == GpuResourceRangeType::cbv)
//                {
//                    const auto& cbufferDesc = rootParamInfo.boundShader->describeConstantBuffer();
//                    const auto cbuffer = renderSystem_->createConstantBuffer(cbufferDesc->getSize());
//                    for (const auto var : cbufferDesc->describeVariables())
//                    {
//                        if (var.second.defaultValue)
//                        {
//                            const auto p = var.second.defaultValue.get();
//                            cbuffer->update(p, var.second.offset, var.second.size);
//                        }
//                    }
//
//                    const auto& constantMap = rootParamInfo.boundShader->getConstantMap();
//                    for (const auto& pair : constantMap)
//                    {
//                        detail::ConstantUpdateInfo updateInfo;
//                        updateInfo.desc = *cbufferDesc->describeVariable(pair.second);
//                        updateInfo.dest = cbuffer;
//                        constantUpdateInfoMap_.emplace(pair.first, updateInfo);
//                    }
//
//                    renderSystem_->createGpuResourceView(heap_cbv_srv_, rangeInfo.offsetInHeap, cbuffer);
//                    rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::cbv,
//                        rangeInfo.baseRegister, rangeInfo.numResources, rangeInfo.offsetInHeap);
//                }
//
//                // Initialize as the texture
//                else if (rangeInfo.type == GpuResourceRangeType::srv)
//                {
//                    rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::srv,
//                        rangeInfo.baseRegister, rangeInfo.numResources, rangeInfo.offsetInHeap);
//                }
//            }
//
//            const auto& textureMap = rootParamInfo.boundShader->getTextureMap();
//            for (const auto& pair : textureMap)
//            {
//                const auto& rangeInfo = rootParamInfo.rangeInfos[rootParamInfo.resourceToRange.at(pair.second)];
//                detail::TextureUpdateInfo updateInfo;
//                updateInfo.tex_index = rangeInfo.offsetInHeap;
//                textureUpdateInfoMap_.emplace(pair.first, updateInfo);
//            }
//        }
//
//        for (size_t rootParamIndex = numRootParams_cbv_srv; rootParamIndex < numRootParams; ++rootParamIndex)
//        {
//            const auto& rootParamInfo = rootParamInfos[rootParamIndex];
//            rootSigDesc[rootParamIndex].asTable(rootParamInfo.numRanges, rootParamInfo.visible);
//            resourceHeapTables_.emplace(rootParamIndex, heap_sampler_);
//
//            for (size_t rangeIndex = 0; rangeIndex < rootParamInfo.numRanges; ++rangeIndex)
//            {
//                const auto& rangeInfo = rootParamInfo.rangeInfos[rangeIndex];
//                assert(rangeInfo.type == GpuResourceRangeType::sampler && "Logic error.");
//                rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::sampler,
//                    rangeInfo.baseRegister, rangeInfo.numResources, rangeInfo.offsetInHeap);
//            }
//
//            const auto& samplerMap = rootParamInfo.boundShader->getSamplerMap();
//            for (const auto& pair : samplerMap)
//            {
//                const auto& rangeInfo = rootParamInfo.rangeInfos[rootParamInfo.resourceToRange.at(pair.second)];
//                auto& updateInfo = textureUpdateInfoMap_.at(pair.first);
//                updateInfo.sam_index = rangeInfo.offsetInHeap;
//                updateInfo.sam_current = std::make_shared<Sampler>();
//                renderSystem_->createGpuResourceView(heap_sampler_, updateInfo.sam_index, updateInfo.sam_current);
//            }
//        }
//
//        PipelineStateDescription pipelineDesc;
//        pipelineDesc.rootSignature = renderSystem_->createRootSignature(rootSigDesc);
//        pipelineDesc.vertexShader = vsRef_->getReferenceVertexShader();
//        pipelineDesc.pixelShader = psRef_->getReferencePixelShader();
//        pipelineDesc.blend = creation.blendState;
//        pipeline_ = renderSystem_->createPipelineState(pipelineDesc);
//    }
//
//    bool EffectPass::forEachLight() const
//    {
//        return forEachLight_;
//    }
//
//    void EffectPass::updateConstant(const std::string& param, const void* data)
//    {
//        const auto it = constantUpdateInfoMap_.find(param);
//        if (it != std::cend(constantUpdateInfoMap_))
//        {
//            const auto size = it->second.desc.size;
//            const auto offset = it->second.desc.offset;
//            it->second.dest->update(data, offset, size);
//        }
//    }
//
//    void EffectPass::updateTexture(const std::string& param, const Resource<Texture>& tex)
//    {
//        const auto it = textureUpdateInfoMap_.find(param);
//        if (it != std::cend(textureUpdateInfoMap_))
//        {
//            renderSystem_->createGpuResourceView(heap_cbv_srv_, it->second.tex_index, tex.access());
//            it->second.tex_current = tex;
//        }
//    }
//
//    void EffectPass::updateSampler(const std::string& param, const std::shared_ptr<Sampler>& sam)
//    {
//        const auto it = textureUpdateInfoMap_.find(param);
//        if (it != std::cend(textureUpdateInfoMap_))
//        {
//            renderSystem_->createGpuResourceView(heap_sampler_, it->second.sam_index, sam);
//            it->second.sam_current = sam;
//        }
//    }
//
//    std::shared_ptr<PipelineState> EffectPass::getPipelineState()
//    {
//        return pipeline_;
//    }
//}