#include "rendertarget.h"

namespace killme
{
    RenderTarget::RenderTarget(ID3D12Resource* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE view)
        : renderTarget_(makeComUnique(renderTarget))
        , view_(view)
    {
    }

    ID3D12Resource* RenderTarget::getD3DRenderTarget()
    {
        return renderTarget_.get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget::getD3DView()
    {
        return view_;
    }
}