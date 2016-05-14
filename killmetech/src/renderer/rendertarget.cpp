#include "rendertarget.h"
#include "texture.h"
#include <cassert>

namespace killme
{
    void RenderTarget::initialize(const std::shared_ptr<Texture>& tex)
    {
        tex_ = tex;
        desc_ = tex_->describeD3D();
    }

    ID3D12Resource* RenderTarget::getD3DResource()
    {
        return tex_->getD3DResource();
    }

    RenderTarget::Location RenderTarget::locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
    {
        device->CreateRenderTargetView(tex_->getD3DResource(), nullptr, location);
        return{ location, desc_.Format };
    }

    std::shared_ptr<RenderTarget> renderTargetInterface(const std::shared_ptr<Texture>& tex)
    {
        assert(tex->describeD3D().Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET &&
            "This texture can not use as the render target.");
        return createRenderDeviceChild<RenderTarget>(tex->getOwnerDevice(), tex);
    }
}