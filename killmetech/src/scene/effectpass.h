#ifndef _KILLME_EFFECTPASS_H_
#define _KILLME_EFFECTPASS_H_

#include "../renderer/shaders.h"
#include "../resources/resource.h"
#include "../core/utility.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace killme
{
    struct PassDescription;
    class MaterialDescription;
    class RenderDevice;
    class PipelineState;
    class GpuResourceHeap;
    class VertexShader;
    class PixelShader;
    class ConstantBuffer;
    class ResourceManager;
    class Texture;
    class Sampler;

    namespace detail
    {
        struct ConstantUpdateInfo
        {
            VariableDescription desc;
            std::shared_ptr<ConstantBuffer> dest;
        };
    
        struct TextureUpdateInfo
        {
            size_t index;
            std::shared_ptr<GpuResourceHeap> dest;
        };

        struct SamplerUpdateInfo
        {
            size_t index;
            std::shared_ptr<GpuResourceHeap> dest;
        };
    }

    /** Light iteration types */
    enum class LightIteration
    {
        none,
        directional,
        point
    };

    /** Effect pass */
    class EffectPass
    {
    private:
        std::shared_ptr<PipelineState> pipeline_;
        std::unordered_multimap<std::string, detail::ConstantUpdateInfo> constantUpdateInfoMap_;
        std::unordered_multimap<std::string, detail::TextureUpdateInfo> textureUpdateInfoMap_;
        std::unordered_multimap<std::string, detail::SamplerUpdateInfo> samplerUpdateInfoMap_;
        LightIteration lightIteration_;

    public:
        /** Construct */
        EffectPass(RenderDevice& device, ResourceManager& resources,
            const MaterialDescription& matDesc, const PassDescription& passDesc);

        /** Retrun light iteration */
        LightIteration getLightIteration() const;
        
        /** Update constant */
        void updateConstant(const std::string& matParam, const void* data, size_t size);
        
        /** Update texture */
        void updateTexture(const std::string& matParam, const Resource<Texture>& tex);
        
        /** Update sampler */
        void updateSampler(const std::string& matParam, const std::shared_ptr<Sampler>& sam);
        
        /** Return pipeline state */
        std::shared_ptr<PipelineState> getPipelineState();
    };
}

#endif