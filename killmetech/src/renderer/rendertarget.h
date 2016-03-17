#ifndef _KILLME_RENDERTARGET_H_
#define _KILLME_RENDERTARGET_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /** Render target */
    class RenderTarget
    {
    private:
        ComUniquePtr<ID3D12Resource> renderTarget_;

    public:
        /** Resource view */
        struct View
        {
            D3D12_CPU_DESCRIPTOR_HANDLE d3dView;
        };

        /** Construct with a Direct3D render target */
        explicit RenderTarget(ID3D12Resource* renderTarget)
            : renderTarget_(makeComUnique(renderTarget))
        {
        }

        /** Return the Direct3D render target */
        ID3D12Resource* getD3DResource() { return renderTarget_.get(); }

        /** Create the Direct3D view into a desctipror heap */
        View createD3DView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            device->CreateRenderTargetView(renderTarget_.get(), nullptr, location);
            return{ location };
        }
    };
}

#endif