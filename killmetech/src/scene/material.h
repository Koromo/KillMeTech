#ifndef _KILLME_MATERIAL_H_
#define _KILLME_MATERIAL_H_

#include "../renderer/shader.h"
#include "../renderer/constantbuffer.h"
#include "../core/optional.h"
#include "../core/utility.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

namespace killme
{
    class GpuResourceHeap;
    class VertexShader;
    class PixelShader;
    class PipelineState;
    class RootSignature;

    /** Material */
    class Material
    {
    private:
        std::shared_ptr<PipelineState> pipelineState_;
        std::shared_ptr<GpuResourceHeap> cbufferHeap_;
        std::unordered_map<std::string, size_t> indexMap_; // Index in heap
        Optional<ConstantBufferDescription> vsParamDesc_;
        Optional<ConstantBufferDescription> psParamDesc_;
        std::shared_ptr<ConstantBuffer> vsParamBuffer_;
        std::shared_ptr<ConstantBuffer> psParamBuffer_;

    public:
        /** Constructs with each shaders */
        Material(const std::shared_ptr<VertexShader>& vs, const std::shared_ptr<PixelShader>& ps);

        /** Store constant buffer into heap */
        void storeConstantBuffer(const std::string& name, const std::shared_ptr<ConstantBuffer>& buffer);

        /** Sets variable parameter */
        template <class T>
        void setVariable(const std::string& name, const T& value)
        {
            if (const auto var = vsParamDesc_ ? vsParamDesc_->describeVariable(name) : nullopt)
            {
                const auto size = var->size;
                const auto offset = var->offset;
                vsParamBuffer_->update(&value, offset, size);
            }
            else if (const auto var = psParamDesc_ ? psParamDesc_->describeVariable(name) : nullopt)
            {
                const auto size = var->size;
                const auto offset = var->offset;
                psParamBuffer_->update(&value, offset, size);
            }
        }

        /** Returns pipeline state */
        std::shared_ptr<PipelineState> getPipelineState();

        /** Returns constant buffer heap */
        std::shared_ptr<GpuResourceHeap> getConstantBufferHeap();

        /** Returns bind tables that are pair of root parameter index and heap */
        auto getConstantBufferHeapTables()
            -> std::unordered_map<size_t, std::shared_ptr<GpuResourceHeap>>
        {
            return{ std::make_pair(0, cbufferHeap_), std::make_pair(1, cbufferHeap_) };
        }
    };
}

#endif