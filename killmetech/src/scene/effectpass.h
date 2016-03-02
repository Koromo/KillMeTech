#ifndef _KILLME_EFFECTPASS_H_
#define _KILLME_EFFECTPASS_H_

#include "../renderer/shader.h"
#include "../resources/resource.h"
#include "../core/utility.h"
#include <memory>
#include <unordered_map>
#include <set>
#include <string>

namespace killme
{
    struct PassDescription;
    class MaterialDescription;
    class RenderSystem;
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

    /** Effect pass */
    class EffectPass
    {
    private:
        std::shared_ptr<RenderSystem> renderSystem_;
        std::shared_ptr<PipelineState> pipeline_;
        std::set<std::shared_ptr<GpuResourceHeap>> resourceHeaps_;
        std::unordered_map<size_t, std::shared_ptr<GpuResourceHeap>> resourceHeapTables_;
        std::unordered_multimap<std::string, detail::ConstantUpdateInfo> constantUpdateInfoMap_;
        std::unordered_multimap<std::string, detail::TextureUpdateInfo> textureUpdateInfoMap_;
        std::unordered_multimap<std::string, detail::SamplerUpdateInfo> samplerUpdateInfoMap_;
        bool forEachLight_;

    public:
        /** Construct */
        EffectPass(const std::shared_ptr<RenderSystem>& renderSystem, ResourceManager& resourceManager,
            const MaterialDescription& matDesc, const PassDescription& passDesc);

        /** Whether this is lighting pass */
        bool forEachLight() const;
        
        /** Update constant */
        void updateConstant(const std::string& matParam, const void* data, size_t size);
        
        /** Update texture */
        void updateTexture(const std::string& matParam, const Resource<Texture>& tex);
        
        /** Update sampler */
        void updateSampler(const std::string& matParam, const std::shared_ptr<Sampler>& sam);
        
        /** Return pipeline state */
        std::shared_ptr<PipelineState> getPipelineState();
        
        /** Return the resource heaps */
        auto getGpuResourceHeaps()
            -> decltype(makeRange(resourceHeaps_))
        {
            return makeRange(resourceHeaps_);
        }
        
        /** Return bind tables that are pair of the root parameter index and the heap */
        auto getGpuResourceHeapTables()
            -> decltype(makeRange(resourceHeapTables_))
        {
            return makeRange(resourceHeapTables_);
        }
    };
}

#endif