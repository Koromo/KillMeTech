#include "pipelinestate.h"

namespace killme
{
    PipelineState::PipelineState(ID3D12PipelineState* state)
        : state_(makeComUnique(state))
    {
    }

    ID3D12PipelineState* PipelineState::getD3DPipelineState()
    {
        return state_.get();
    }
}