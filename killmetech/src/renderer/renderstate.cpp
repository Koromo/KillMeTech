#include "renderstate.h"

namespace killme
{
    const BlendState BlendState::DEFAULT = {
        false,
        Blend::one,
        Blend::zero,
        BlendOp::add
    };
}