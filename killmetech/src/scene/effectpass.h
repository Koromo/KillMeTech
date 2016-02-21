#ifndef _KILLME_EFFECTPASS_H_
#define _KILLME_EFFECTPASS_H_

#include "../core/utility.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace killme
{
    class EffectShaderRef;
    class RenderSystem;
    class PipelineState;
    class GpuResourceHeap;

    /** Effect pass */
    class EffectPass
    {
    private:
        std::shared_ptr<PipelineState> pipeline_;
        std::shared_ptr<GpuResourceHeap> resourceHeap_;
        std::unordered_map<size_t, std::shared_ptr<GpuResourceHeap>> resourceHeapTable_;
        std::shared_ptr<EffectShaderRef> vsRef_;
        std::shared_ptr<EffectShaderRef> psRef_;

    public:
        /** Construct */
        EffectPass(RenderSystem& renderSystem,
            const std::shared_ptr<EffectShaderRef>& vsRef, const std::shared_ptr<EffectShaderRef>& psRef);

        /** Update constant */
        void updateConstant(const std::string& param, const void* data);

        /** Return pipeline state */
        std::shared_ptr<PipelineState> getPipelineState();

        /** Return the resource heaps */
        auto getGpuResourceHeaps()
            -> decltype(makeRange(std::vector<std::shared_ptr<GpuResourceHeap>>()))
        {
            std::vector<std::shared_ptr<GpuResourceHeap>> heaps;
            if (resourceHeap_)
            {
                heaps.emplace_back(resourceHeap_);
            }
            return makeRange(std::move(heaps));
        }

        /** Return bind tables that are pair of the root parameter index and the heap */
        auto getGpuResourceHeapTables()
            -> decltype(makeRange(resourceHeapTable_))
        {
            return makeRange(resourceHeapTable_);
        }
    };
}

#endif