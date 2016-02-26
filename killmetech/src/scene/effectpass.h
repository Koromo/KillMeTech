#ifndef _KILLME_EFFECTPASS_H_
#define _KILLME_EFFECTPASS_H_

#include "../renderer/shader.h"
#include "../renderer/renderstate.h"
#include "../resources/resource.h"
#include "../core/utility.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace killme
{
    class EffectShaderRef;
    class RenderSystem;
    class PipelineState;
    class GpuResourceHeap;
    class ConstantBuffer;
    class Texture;
    class Sampler;

    struct EffectPassCreation
    {
        std::shared_ptr<EffectShaderRef> vsRef;
        std::shared_ptr<EffectShaderRef> psRef;
        bool forEachLight;
        BlendState blendState;
    };

    namespace detail
    {
        struct ConstantUpdateInfo
        {
            VariableDescription desc;
            std::shared_ptr<ConstantBuffer> dest;
        };

        struct TextureUpdateInfo
        {
            size_t tex_index;
            Resource<Texture> tex_current;

            size_t sam_index;
            std::shared_ptr<Sampler> sam_current;
        };
    }

    /** Effect pass */
    class EffectPass
    {
    private:
        std::shared_ptr<RenderSystem> renderSystem_;
        std::shared_ptr<PipelineState> pipeline_;
        std::shared_ptr<GpuResourceHeap> heap_cbv_srv_;
        std::shared_ptr<GpuResourceHeap> heap_sampler_;
        std::unordered_map<size_t, std::shared_ptr<GpuResourceHeap>> resourceHeapTables_;
        std::unordered_map<std::string, detail::ConstantUpdateInfo> constantUpdateInfoMap_;
        std::unordered_map<std::string, detail::TextureUpdateInfo> textureUpdateInfoMap_;
        std::shared_ptr<EffectShaderRef> vsRef_;
        std::shared_ptr<EffectShaderRef> psRef_;
        bool forEachLight_;

    public:
        /** Construct */
        EffectPass(const std::shared_ptr<RenderSystem>& renderSystem, const EffectPassCreation& creation);

        /** Whether this is lighting pass */
        bool forEachLight() const;

        /** Update constant */
        void updateConstant(const std::string& param, const void* data);

        /** Update texture */
        void updateTexture(const std::string& param, const Resource<Texture>& tex);

        /** Update sampler */
        void updateSampler(const std::string& param, const std::shared_ptr<Sampler>& sam);

        /** Return pipeline state */
        std::shared_ptr<PipelineState> getPipelineState();

        /** Return the resource heaps */
        auto getGpuResourceHeaps()
            -> decltype(makeRange(std::vector<std::shared_ptr<GpuResourceHeap>>()))
        {
            std::vector<std::shared_ptr<GpuResourceHeap>> heaps;
            if (heap_cbv_srv_)
            {
                heaps.emplace_back(heap_cbv_srv_);
            }
            if (heap_sampler_)
            {
                heaps.emplace_back(heap_sampler_);
            }
            return makeRange(std::move(heaps));
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