#include "depthstencil.h"
#include "texture.h"
#include <cassert>

namespace killme
{
    void DepthStencil::initialize(const std::shared_ptr<Texture>& tex)
    {
        tex_ = tex;
        desc_ = tex_->describeD3D();
    }

    DepthStencil::Location DepthStencil::locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc;
        viewDesc.Format = desc_.Format;
        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Flags = D3D12_DSV_FLAG_NONE;
        device->CreateDepthStencilView(tex_->getD3DResource(), &viewDesc, location);
        return{ location, desc_.Format };
    }

    std::shared_ptr<DepthStencil> depthStencilInterface(const std::shared_ptr<Texture>& tex)
    {
        assert(tex->describeD3D().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL &&
            "This texture can not use as the depth stencil.");
        return createRenderDeviceChild<DepthStencil>(tex->getOwnerDevice(), tex);
    }
}