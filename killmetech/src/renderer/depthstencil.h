#ifndef _KILLME_DEPTHSTENCIL_H_
#define _KILLME_DEPTHSTENCIL_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /** Depth stencil */
    /// TODO: Not supported stencil buffer
    class DepthStencil
    {
    private:
        ComUniquePtr<ID3D12Resource> depthStencil_;
        DXGI_FORMAT format_;
        D3D12_CPU_DESCRIPTOR_HANDLE view_;

    public:
        /** Constructs with a Direct3D depth stencil */
        DepthStencil(ID3D12Resource* depthStencil, DXGI_FORMAT format)
            : depthStencil_(makeComUnique(depthStencil))
            , format_(format)
            , view_()
        {
        }

        /** Returns a Direct3D view */
        D3D12_CPU_DESCRIPTOR_HANDLE getD3DView() { return view_; }

        /** Creates a Direct3D view to desctipror heap */
        void createD3DView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc;
            viewDesc.Format = format_;
            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
            viewDesc.Flags = D3D12_DSV_FLAG_NONE;

            device->CreateDepthStencilView(depthStencil_.get(), &viewDesc, location);
            view_ = location;
        }
    };
}

#endif