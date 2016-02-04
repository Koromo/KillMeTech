#ifndef _KILLME_RENDERTARGET_H_
#define _KILLME_RENDERTARGET_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /// TODO: Now, Resource barrior used by only render target transition
    /** For render target barrior */
    enum class ResourceState
    {
        present,
        renderTarget
    };

    /** Render target resource */
    class RenderTarget
    {
    private:
        ComUniquePtr<ID3D12Resource> renderTarget_;
        D3D12_CPU_DESCRIPTOR_HANDLE view_;

    public:
        /** Construct */
        explicit RenderTarget(ID3D12Resource* renderTarget)
            : renderTarget_(makeComUnique(renderTarget))
            , view_()
        {
        }

        /** Returns Direct3D render target */
        ID3D12Resource* getD3DRenderTarget() { return renderTarget_.get(); }

        /** Returns Direct3D view */
        D3D12_CPU_DESCRIPTOR_HANDLE getD3DView() { return view_; }

        /** Store resource to heap */
        void createView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            device->CreateRenderTargetView(renderTarget_.get(), nullptr, location);
            view_ = location;
        }
    };
}

#endif