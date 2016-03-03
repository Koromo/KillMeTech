#ifndef _KILLME_PIPELINESTATE_H_
#define _KILLME_PIPELINESTATE_H_

#include "renderstate.h"
#include "../resources/resource.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    class RootSignature;
    class VertexShader;
    class PixelShader;
    class GeometryShader;

    /** For PipelineState creation */
    struct PipelineStateDescription
    {
        std::shared_ptr<RootSignature> rootSignature;
        Resource<VertexShader> vertexShader;
        Resource<PixelShader> pixelShader;
        Resource<GeometryShader> geometryShader;
        BlendState blend;
    };

    /** Pipeline state */
    class PipelineState
    {
    private:
        ComUniquePtr<ID3D12PipelineState> state_;
        PipelineStateDescription desc_;

    public:
        /** Construct with a Direct3D pipeline state and the description */
        PipelineState(ID3D12PipelineState* state, const PipelineStateDescription& desc)
            : state_(makeComUnique(state))
            , desc_(desc) {}

        /** Return the Direct3D pipeline state */
        ID3D12PipelineState* getD3DPipelineState() { return state_.get(); }

        /** Return the description */
        PipelineStateDescription describe() const { return desc_; }
    };
}

#endif