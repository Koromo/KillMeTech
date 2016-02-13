#ifndef _KILLME_MATERIAL_H_
#define _KILLME_MATERIAL_H_

#include "../renderer/shader.h"
#include "../renderer/constantbuffer.h"
#include "../core/optional.h"
#include "../resource/resource.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace killme
{
    class GpuResourceHeap;
    class VertexShader;
    class PixelShader;
    class PipelineState;
    class RootSignature;

    /** The material */
    class Material : public IsResource
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
        Material(const Resource<VertexShader>& vs, const Resource<PixelShader>& ps);

        /** Copy material */
        /// TOOD: Copy is inefficiency
        Material(const Material& lhs);

        /** Stores a constant buffer into the heap */
        void storeConstantBuffer(const std::string& name, const std::shared_ptr<ConstantBuffer>& buffer);

        /** Sets a variable parameter */
        template <class T>
        void setVariable(const std::string& name, const T& value)
        {
            setVariableImpl(name, &value);
        }

        /** Returns the pipeline state */
        std::shared_ptr<PipelineState> getPipelineState();

        /** Returns the constant buffer heap */
        std::shared_ptr<GpuResourceHeap> getConstantBufferHeap();

        /** Returns bind tables that are pair of the root parameter index and the heap */
        auto getConstantBufferHeapTables()
            -> std::unordered_map<size_t, std::shared_ptr<GpuResourceHeap>>
        {
            return{ std::make_pair(0, cbufferHeap_), std::make_pair(1, cbufferHeap_) };
        }

    private:
        void setVariableImpl(const std::string& name, const void* data);
    };

    std::shared_ptr<Material> loadMaterial(const std::string& path);
}

#endif