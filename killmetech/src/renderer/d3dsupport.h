#ifndef _KILLME_D3DSUPPORT_H_
#define _KILLME_D3DSUPPORT_H_

#include "../core/exception.h"
#include <d3d12.h>
#include <string>

namespace killme
{
    /** The exception of Direct3D */
    class Direct3DException : public Exception
    {
    public:
        /** Constructs with a message */
        explicit Direct3DException(const std::string& msg);
    };

    /** Returns the default heap properties */
    D3D12_HEAP_PROPERTIES getD3DDefaultHeapProps();

    /** Returns the upload heap properties */
    D3D12_HEAP_PROPERTIES getD3DUploadHeapProps();

    /** Returns the resource description about buffers */
    D3D12_RESOURCE_DESC describeD3DBuffer(size_t size);

    /** Returns the resource description about 2D textures */
    D3D12_RESOURCE_DESC describeD3DTex2D(size_t width, size_t height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags);
}

#endif
