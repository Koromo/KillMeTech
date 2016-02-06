#ifndef _KILLME_PIPELINESTATE_H_
#define _KILLME_PIPELINESTATE_H_

#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    class RootSignature;
    class VertexShader;
    class PixelShader;

    /** For the PipelineState creation */
    struct PipelineStateDescription
    {
        std::shared_ptr<RootSignature> rootSignature;
        std::shared_ptr<const VertexShader> vertexShader;
        std::shared_ptr<const PixelShader> pixelShader;
    };

    /** The pipeline state */
    class PipelineState
    {
    private:
        ComUniquePtr<ID3D12PipelineState> state_;
        PipelineStateDescription desc_;

    public:
        /** Constructs with a Direct3D pipeline state and the description */
        PipelineState(ID3D12PipelineState* state, const PipelineStateDescription& desc)
            : state_(makeComUnique(state))
            , desc_(desc) {}

        /** Returns the Direct3D pipeline state */
        ID3D12PipelineState* getD3DPipelineState() { return state_.get(); }

        /** Returns the description */
        PipelineStateDescription describe() const { return desc_; }
    };
}

#endif