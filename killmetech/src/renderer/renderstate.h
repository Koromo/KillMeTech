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
}

#endif