#ifndef _KILLME_RENDERTARGET_H_
#define _KILLME_RENDERTARGET_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /** For render target barrior */
    enum class ResourceState
    {
        present,
        renderTarget
    };

    /** Render target */
    class RenderTarget
    {
    private:
        ComUniquePtr<ID3D12Resource> renderTarget_;
        D3D12_CPU_DESCRIPTOR_HANDLE view_;

    public:
        /** Constructs with a Direct3D render target */
        explicit RenderTarget(ID3D12Resource* renderTarget)
            : renderTarget_(makeComUnique(renderTarget))
            , view_()
        {
        }

        /** Returns a Direct3D render target */
        ID3D12Resource* getD3DRenderTarget() { return renderTarget_.get(); }

        /** Returns a Direct3D view */
        D3D12_CPU_DESCRIPTOR_HANDLE getD3DView() { return view_; }

        /** Creates a Direct3D view to desctipror heap */
        void createD3DView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            device->CreateRenderTargetView(renderTarget_.get(), nullptr, location);
            view_ = location;
        }
    };
}

#endif