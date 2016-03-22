#include "effectpass.h"
#include "materialcreation.h"
#include "../renderer/texture.h"
#include "../renderer/renderdevice.h"
#include "../renderer/gpuresource.h"
#include "../renderer/pipelinestate.h"
#include "../renderer/constantbuffer.h"
#include <utility>
#include <algorithm>

#undef min

namespace killme
{
    /// TODO: Output errors
    EffectPass::EffectPass(RenderDevice& device, ResourceManager& resources,
        const MaterialDescription& matDesc, const PassDescription& passDesc)
        : pipeline_(device.createPipelineState())
        , constantUpdateInfoMap_()
        , textureUpdateInfoMap_()
        , samplerUpdateInfoMap_()
        , lightIteration_(passDesc.lightIteration)
    {
        std::unordered_map<std::shared_ptr<const BasicShader>, ShaderBoundDescription> eachShaders;
        for (const auto& ref : passDesc.shaderRef)
        {
            const auto& bound = matDesc.getShaderBound(ref.first, ref.second);
            if (ref.first == ShaderType::vertex)
            {
                const Resource<VertexShader> shader(resources, bound.path);
                pipeline_->setVShader(shader);
                eachShaders.emplace(shader.access(), bound);
            }
            else if (ref.first == ShaderType::pixel)
            {
                const Resource<PixelShader> shader(resources, bound.path);
                pipeline_->setPShader(shader);
                eachShaders.emplace(shader.access(), bound);
            }
            else if (ref.first == ShaderType::geometry)
            {
                const Resource<GeometryShader> shader(resources, bound.path);
                pipeline_->setGShader(shader);
                eachShaders.emplace(shader.access(), bound);
            }
        }

        pipeline_->setBlendState(0, passDesc.blendState);

        const auto resourceTable = pipeline_->getGpuResourceTable();
        const auto numHeaps = resourceTable->getNumRequiredHeaps();
        for (size_t i = 0; i < numHeaps; ++i)
        {
            const auto& requiredHeap = resourceTable->getRequiredHeap(i);
            const auto numResources = requiredHeap.getNumResources();
            const auto heap = device.createGpuResourceHeap(numResources, requiredHeap.getType(), true);

            for (size_t j = 0; j < numResources; ++j)
            {
                const auto& requiredResource = requiredHeap.getResource(j);
                const auto& boundDesc = eachShaders[requiredResource.boundShader.lock()];

                if (requiredResource.type == BoundResourceType::cbuffer)
                {
                    const auto cbuffer = device.createConstantBuffer(requiredResource.cbuffer.getSize());

                    for (const auto& var : requiredResource.cbuffer.describeVariables())
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

                    heap->locate(j, cbuffer);
                }
                else if (requiredResource.type == BoundResourceType::texture)
                {
                    const auto it = boundDesc.textureMapping.find(requiredResource.texture.getName());
                    if (it != std::cend(boundDesc.textureMapping))
                    {
                        detail::TextureUpdateInfo update;
                        update.index = j;
                        update.dest = heap;
                        textureUpdateInfoMap_.emplace(it->second, std::move(update));
                    }
                }
                else if (requiredResource.type == BoundResourceType::sampler)
                {
                    const auto it = boundDesc.samplerMapping.find(requiredResource.sampler.getName());
                    if (it != std::cend(boundDesc.samplerMapping))
                    {
                        detail::SamplerUpdateInfo update;
                        update.index = j;
                        update.dest = heap;
                        samplerUpdateInfoMap_.emplace(it->second, std::move(update));
                    }
                }
            }

            for (const auto rootIndex : requiredHeap.getRootIndices())
            {
                resourceTable->set(rootIndex, heap);
            }
        }
    }

    LightIteration EffectPass::getLightIteration() const
    {
        return lightIteration_;
    }

    void EffectPass::updateConstant(const std::string& matParam, const void* data, size_t size)
    {
        auto range = constantUpdateInfoMap_.equal_range(matParam);
        while (range.first != range.second)
        {
            const auto& update = range.first->second;
            update.dest->update(data, update.desc.offset, std::min(size, update.desc.size));
            ++range.first;
        }
    }

    void EffectPass::updateTexture(const std::string& matParam, const Resource<Texture>& tex)
    {
        auto range = textureUpdateInfoMap_.equal_range(matParam);
        while (range.first != range.second)
        {
            const auto& update = range.first->second;
            update.dest->locate(update.index, tex.access());
            ++range.first;
        }
    }

    void EffectPass::updateSampler(const std::string& matParam, const std::shared_ptr<Sampler>& sam)
    {
        auto range = samplerUpdateInfoMap_.equal_range(matParam);
        while (range.first != range.second)
        {
            const auto& update = range.first->second;
            update.dest->locate(update.index, sam);
            ++range.first;
        }
    }

    std::shared_ptr<PipelineState> EffectPass::getPipelineState()
    {
        return pipeline_;
    }
}