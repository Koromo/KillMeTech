#ifndef _KILLME_PIPELINESTATE_H_
#define _KILLME_PIPELINESTATE_H_

#include "shader.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    class RootSignature;

    /** For PipelineState creation */
    struct PipelineStateDescription
    {
        std::shared_ptr<RootSignature> rootSignature;
        std::shared_ptr<const VertexShader> vertexShader;
        std::shared_ptr<const PixelShader> pixelShader;
    };

    /** Pipeline state */
    class PipelineState
    {
    private:
        ComUniquePtr<ID3D12PipelineState> state_;

    public:
        /** Construct with a Direct3D pipeline state */
        explicit PipelineState(ID3D12PipelineState* state);

        /** Returns Direct3D pipeline state */
        ID3D12PipelineState* getD3DPipelineState();
    };
}

#endif
