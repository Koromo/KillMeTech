#ifndef _KILLME_DEPTHSTENCIL_H_
#define _KILLME_DEPTHSTENCIL_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /** The depth stencil */
    /// TODO: Not supported stencil buffer
    class DepthStencil
    {
    private:
        ComUniquePtr<ID3D12Resource> depthStencil_;
        DXGI_FORMAT format_;

    public:
        /** The resource view */
        struct View
        {
            D3D12_CPU_DESCRIPTOR_HANDLE d3dView;
        };

        /** Constructs with a Direct3D depth stencil */
        explicit DepthStencil(ID3D12Resource* depthStencil)
            : depthStencil_(makeComUnique(depthStencil))
            , format_(depthStencil->GetDesc().Format)
        {
        }

        /** Creates the Direct3D view into a desctipror heap */
        View createD3DView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc;
            viewDesc.Format = format_;
            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
            viewDesc.Flags = D3D12_DSV_FLAG_NONE;
            device->CreateDepthStencilView(depthStencil_.get(), &viewDesc, location);
            return{ location };
        }
    };
}

#endif