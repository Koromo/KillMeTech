#ifndef _KILLME_RENDERTARGET_H_
#define _KILLME_RENDERTARGET_H_

#include "renderdevice.h"
#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /** Render target */
    class RenderTarget : public RenderDeviceChild
    {
    private:
        ComSharedPtr<ID3D12Resource> texture_;
        D3D12_RESOURCE_DESC desc_;

    public:
        /** Resource location */
        struct Location
        {
            D3D12_CPU_DESCRIPTOR_HANDLE ofD3D;
            DXGI_FORMAT format;
        };

        /** Initialize */
        void initialize(ID3D12Resource* texture);
        void initialize(const ComSharedPtr<ID3D12Resource>& texture);

        /** Return the pixel format */
        PixelFormat getPixelFormat() const;

        /** Return the Direct3D render target */
        ID3D12Resource* getD3DResource();

        /** Create the Direct3D view into a desctipror heap */
        Location locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location);
    };
}

#endif