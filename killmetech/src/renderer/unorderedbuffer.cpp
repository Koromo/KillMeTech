#include "unorderedbuffer.h"
#include "texture.h"
#include <cassert>

namespace killme
{
    void UnorderedBuffer::initialize(const std::shared_ptr<Texture>& tex)
    {
        tex_ = tex;
        desc_ = tex_->describeD3D();
    }

    UnorderedBuffer::Location UnorderedBuffer::locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc;
        viewDesc.Format = desc_.Format;
        viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Texture2D.PlaneSlice = 0;
        device->CreateUnorderedAccessView(tex_->getD3DResource(), nullptr, &viewDesc, location);
        return{ location, desc_.Format };
    }

    std::shared_ptr<UnorderedBuffer> unorderedBufferInterface(const std::shared_ptr<Texture>& tex)
    {
        assert(tex->describeD3D().Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS &&
            "This texture can not use as the render target.");
        return createRenderDeviceChild<UnorderedBuffer>(tex->getOwnerDevice(), tex);
    }
}