#ifndef _KILLME_DEPTHSTENCIL_H_
#define _KILLME_DEPTHSTENCIL_H_

#include "renderdevice.h"
#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    enum class PixelFormat;

    /** Depth stencil */
    /// TODO: Not supported stencil buffer
    class DepthStencil : public RenderDeviceChild
    {
    private:
        ComSharedPtr<ID3D12Resource> texture_;
        D3D12_RESOURCE_DESC desc_;

    public:
        /** Resource location */
        struct Location
        {
            D3D12_CPU_DESCRIPTOR_HANDLE ofD3D;
        };

        /** Initialize */
        void initialize(const ComSharedPtr<ID3D12Resource>& texture);

        /** Return the pixel format */
        PixelFormat getPixelFormat() const;

        /** Create the Direct3D view into a desctipror heap */
        Location locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location);
    };
}

#endif