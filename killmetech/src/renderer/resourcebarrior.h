#ifndef _KILLME_RESOURCEBARRIOR_H_
#define _KILLME_RESOURCEBARRIOR_H_

namespace killme
{
    enum class ResourceState
    {
        present,
        renderTarget,
        copyDestination,
        vertexBuffer,
        indexBuffer,
        texture
    };
}

#endif