#ifndef _KILLME_MATERIAL_H_
#define _KILLME_MATERIAL_H_

#include "../renderer/shader.h"
#include "../renderer/constantbuffer.h"
#include "../resources/resource.h"
#include "../core/optional.h"
#include "../core/utility.h"
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
    class RootSignatureDescription;
    class RenderSystem;
    class ResourceManager;
    struct SceneConstantBuffers;

    /** The material */
    class Material : public IsResource
    {
    private:
        std::shared_ptr<PipelineState> pipeline_;
        std::shared_ptr<GpuResourceHeap> paramHeap_;
        std::unordered_map<size_t, std::shared_ptr<GpuResourceHeap>> paramHeapTable_;
        Optional<ConstantBufferDescription> vsParamDesc_;
        Optional<ConstantBufferDescription> psParamDesc_;
        std::shared_ptr<ConstantBuffer> vsParamBuffer_;
        std::shared_ptr<ConstantBuffer> psParamBuffer_;

    public:
        /** Constructs with each shaders */
        Material(RenderSystem& renderSystem, const Resource<VertexShader>& vs, const Resource<PixelShader>& ps);

        /** Sets a variable parameter */
        template <class T>
        void setVariable(const std::string& name, const T& value)
        {
            setVariableImpl(name, &value);
        }

        /** Returns the pipeline state */
        std::shared_ptr<PipelineState> getPipelineState();

        /** Returns the constant buffer heap */
        auto getConstantBufferHeaps()
            -> decltype(makeRange(std::vector<std::shared_ptr<GpuResourceHeap>>()))
        {
            std::vector<std::shared_ptr<GpuResourceHeap>> heaps = { paramHeap_ };
            return makeRange(std::move(heaps));
        }

        /** Returns bind tables that are pair of the root parameter index and the heap */
        auto getConstantBufferHeapTables()
            -> decltype(makeRange(paramHeapTable_))
        {
            return makeRange(paramHeapTable_);
        }

    private:
        std::shared_ptr<ConstantBuffer> initParams(RenderSystem& renderSystem,
            const ConstantBufferDescription& paramDesc, RootSignatureDescription& rootSigDesc,
            ShaderType visible, size_t rootParamIndex, size_t offsetInHeap);

        void setVariableImpl(const std::string& name, const void* data);
    };

    std::shared_ptr<Material> loadMaterial(RenderSystem& renderSystem, ResourceManager& resourceManager, const std::string& path);
}

#endif