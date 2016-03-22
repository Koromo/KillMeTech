#include "rendertarget.h"
#include "d3dsupport.h"

namespace killme
{
    void RenderTarget::initialize(ID3D12Resource* texture)
    {
        texture_ = makeComShared(texture);
        desc_ = texture_->GetDesc();
    }

    void RenderTarget::initialize(const ComSharedPtr<ID3D12Resource>& texture)
    {
        texture_ = texture;
        desc_ = texture_->GetDesc();
    }

    PixelFormat RenderTarget::getPixelFormat() const
    {
        return D3DMappings::toPixelFormat(desc_.Format);
    }

    ID3D12Resource* RenderTarget::getD3DResource()
    {
        return texture_.get();
    }

    RenderTarget::Location RenderTarget::locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
    {
        device->CreateRenderTargetView(texture_.get(), nullptr, location);
        return{ location, desc_.Format };
    }
}