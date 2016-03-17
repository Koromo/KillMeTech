#include "effectpass.h"
#include "materialcreation.h"
#include "../renderer/shaders.h"
#include "../renderer/texture.h"
#include "../renderer/rendersystem.h"
#include "../renderer/gpuresourceheap.h"
#include "../renderer/rootsignature.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/constantbuffer.h"
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
        std::vector<RootParamInfo> getRootParamInfos(RenderSystem& renderSystem, Shaders shaders)
        {
            std::vector<RootParamInfo> rootParams;
            auto offsetInHeap_cbv_srv = 0;
            auto offsetInHeap_sampler = 0;

            for (const auto& pair : shaders)
            {
                // For cbv_srv
                const auto shader = pair.first;
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
                heap_cbv_srv = renderSystem.createGpuResourceHeap(offsetInHeap_cbv_srv, GpuResourceHeapType::cbv_srv, GpuResourceHeapFlag::shaderVisible);
            }

            std::shared_ptr<GpuResourceHeap> heap_sampler;
            if (offsetInHeap_sampler > 0)
            {
                heap_sampler = renderSystem.createGpuResourceHeap(offsetInHeap_sampler, GpuResourceHeapType::sampler, GpuResourceHeapFlag::shaderVisible);
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

    /// TODO: Output errors
    EffectPass::EffectPass(RenderSystem& renderSystem, ResourceManager& resourceManager,
        const MaterialDescription& matDesc, const PassDescription& passDesc)
        : pipeline_()
        , resourceHeaps_()
        , resourceHeapTables_()
        , constantUpdateInfoMap_()
        , textureUpdateInfoMap_()
        , samplerUpdateInfoMap_()
        , lightIteration_(passDesc.lightIteration)
    {
        // Create pipeline state
        PipelineStateDescription pipelineDesc;
        pipelineDesc.blend = passDesc.blendState;

        // Create shaders
        std::unordered_map<std::shared_ptr<BasicShader>, ShaderBoundDescription> eachShaders;
        for (const auto& ref : passDesc.shaderRef)
        {
            const auto& bound = matDesc.getShaderBound(ref.first, ref.second);
            std::shared_ptr<BasicShader> shaderBase;
            if (ref.first == ShaderType::vertex)
            {
                const Resource<VertexShader> shader(resourceManager, bound.path);
                pipelineDesc.vertexShader = shader;
                eachShaders.emplace(shader.access(), bound);
            }
            else if (ref.first == ShaderType::pixel)
            {
                const Resource<PixelShader> shader(resourceManager, bound.path);
                pipelineDesc.pixelShader = shader;
                eachShaders.emplace(shader.access(), bound);
            }
            else if (ref.first == ShaderType::geometry)
            {
                const Resource<GeometryShader> shader(resourceManager, bound.path);
                pipelineDesc.geometryShader = shader;
                eachShaders.emplace(shader.access(), bound);
            }
        }

        // Create root signature
        const auto rootParamInfos = getRootParamInfos(renderSystem, eachShaders);

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
                    const auto cbuffer = renderSystem.createConstantBuffer(cbufferDesc->getSize());
                    const auto& boundDesc = eachShaders[rootParamInfo.shader];

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
                    rootParamInfo.heap->createView(rangeInfo.offsetInHeap, cbuffer);
                    rootSigDesc[rootParamIndex][rangeIndex].as(GpuResourceRangeType::cbv, cbufferDesc->getRegisterSlot(), 1, rangeInfo.offsetInHeap);
                }
                else if (rangeInfo.type == GpuResourceRangeType::srv) // Texture
                {
                    const auto& boundDesc = eachShaders[rootParamInfo.shader];
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
                    const auto& boundDesc = eachShaders[rootParamInfo.shader];
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

        pipelineDesc.rootSignature = renderSystem.createRootSignature(rootSigDesc);
        pipeline_ = renderSystem.createPipelineState(pipelineDesc);
    }

    LightIteration EffectPass::getLightIteration() const
    {
        return lightIteration_;
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
            it->second.dest->createView(it->second.index, tex.access());
        }
    }

    void EffectPass::updateSampler(const std::string& matParam, const std::shared_ptr<Sampler>& sam)
    {
        const auto it = samplerUpdateInfoMap_.find(matParam);
        if (it != std::cend(samplerUpdateInfoMap_))
        {
            it->second.dest->createView(it->second.index, sam);
        }
    }

    std::shared_ptr<PipelineState> EffectPass::getPipelineState()
    {
        return pipeline_;
    }
}