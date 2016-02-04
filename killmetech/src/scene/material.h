#ifndef _KILLME_MATERIAL_H_
#define _KILLME_MATERIAL_H_

#include <memory>

namespace killme
{
    class PipelineState;

    /** Material */
    class Material
    {
    private:
        std::shared_ptr<PipelineState> pipelineState_;

    public:
        /** Construct with a pipeline state and a root signature */
        Material(const std::shared_ptr<PipelineState>& pipelineState) : pipelineState_(pipelineState) {}

        /** Returns pipeline state */
        std::shared_ptr<PipelineState> getPipelineState() { return pipelineState_; }
    };
}

#endif