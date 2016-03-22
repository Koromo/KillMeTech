#include "depthstencil.h"
#include "d3dsupport.h"

namespace killme
{
    void DepthStencil::initialize(const ComSharedPtr<ID3D12Resource>& texture)
    {
        texture_ = texture;
        desc_ = texture_->GetDesc();
    }

    PixelFormat DepthStencil::getPixelFormat() const
    {
        return D3DMappings::toPixelFormat(desc_.Format);
    }

    DepthStencil::Location DepthStencil::locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc;
        viewDesc.Format = desc_.Format;
        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Flags = D3D12_DSV_FLAG_NONE;
        device->CreateDepthStencilView(texture_.get(), &viewDesc, location);
        return{ location };
    }
}