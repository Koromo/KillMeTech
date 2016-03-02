#ifndef _KILLME_RENDERSTATE_H_
#define _KILLME_RENDERSTATE_H_

namespace killme
{
    /** Viewport */
    struct Viewport
    {
        float width;
        float height;
        float topLeftX;
        float topLeftY;
        float minDepth;
        float maxDepth;
    };

    /** Scissor rect */
    struct ScissorRect
    {
        int top;
        int left;
        int right;
        int bottom;
    };

    /** Blend state */
    enum class Blend
    {
        one,
        zero
    };

    enum class BlendOp
    {
        add,
        subtract,
        min,
        max
    };

    struct BlendState
    {
        bool enable;
        Blend src;
        Blend dest;
        BlendOp op;

        /** Default blend state */
        static const BlendState DEFAULT;
    };
}

#endif