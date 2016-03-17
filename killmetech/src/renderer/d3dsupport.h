#ifndef _KILLME_D3DSUPPORT_H_
#define _KILLME_D3DSUPPORT_H_

#include "../core/exception.h"
#include <d3d12.h>
#include <string>

namespace killme
{
    enum class PixelFormat;

    /** Exception of Direct3D */
    class Direct3DException : public Exception
    {
    public:
        /** Construct with a message */
        explicit Direct3DException(const std::string& msg);
    };

    /** Return the default heap properties */
    D3D12_HEAP_PROPERTIES getD3DDefaultHeapProps();

    /** Return the upload heap properties */
    D3D12_HEAP_PROPERTIES getD3DUploadHeapProps();

    /** Return the resource description about buffers */
    D3D12_RESOURCE_DESC describeD3DBuffer(size_t size);

    /** Return the resource description about 2D textures */
    D3D12_RESOURCE_DESC describeD3DTex2D(size_t width, size_t height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags);

    /** Return DXGI_FORMAT */
    DXGI_FORMAT toD3DPixelFormat(PixelFormat pf);

    /** Update subresources */
    void updateSubresources(ID3D12GraphicsCommandList* commandList, ID3D12Resource* destResource, ID3D12Resource* intermediate,
        size_t intermediateOffset, size_t firstSubresource, size_t numSubresources, const D3D12_SUBRESOURCE_DATA* pSrcData);

    /** Calclate required intermediate size */
    size_t calcRequiredIntermediateSize(ID3D12Resource* dest, size_t intermediateOffset, size_t firstSubresource, size_t numSubresources);
}

#endif
