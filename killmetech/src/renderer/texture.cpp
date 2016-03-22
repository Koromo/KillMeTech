#include "texture.h"
#include "rendertarget.h"
#include "depthstencil.h"
#include "d3dsupport.h"
#include "../core/math/color.h"
#include <Windows.h>
#include <cassert>

namespace killme
{
    void Texture::initialize(size_t width, size_t height, PixelFormat format, TextureFlags flags,
        GpuResourceState initialState, Optional<Color> optimizedClear)
    {
        const auto d3dFormat = D3DMappings::toD3DDxgiFormat(format);
        const auto defaultHeapProps = getD3DDefaultHeapProps();
        const auto desc = describeD3DTex2D(width, height, d3dFormat, D3DMappings::toD3DResourceFlags(flags));

        const D3D12_CLEAR_VALUE* pOptimizedClear = nullptr;
        D3D12_CLEAR_VALUE d3dOptimizedClear;

        if (optimizedClear)
        {
            d3dOptimizedClear.Format = d3dFormat;
            d3dOptimizedClear.Color[0] = optimizedClear->r;
            d3dOptimizedClear.Color[1] = optimizedClear->g;
            d3dOptimizedClear.Color[2] = optimizedClear->b;
            d3dOptimizedClear.Color[3] = optimizedClear->a;
            pOptimizedClear = &d3dOptimizedClear;
        }

        ID3D12Resource* tex;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc,
                D3DMappings::toD3DResourceState(initialState), pOptimizedClear, IID_PPV_ARGS(&tex))),
            "Failed to create the texture.");
        tex_ = makeComShared(tex);

        resourceDesc_ = tex_->GetDesc();
        format_ = format;
    }

    void Texture::initialize(size_t width, size_t height, PixelFormat format, TextureFlags flags,
        GpuResourceState initialState, float optimizedDepth, unsigned optimizedStencil)
    {
        assert((flags & TextureFlags::allowDepthStencil) && "You need to up the bit of TextureFlags::allowDepthStencil.");
        const auto d3dFormat = D3DMappings::toD3DDxgiFormat(format);
        const auto defaultHeapProps = getD3DDefaultHeapProps();
        const auto desc = describeD3DTex2D(width, height, d3dFormat, D3DMappings::toD3DResourceFlags(flags));

        D3D12_CLEAR_VALUE optimizedClear;
        optimizedClear.Format = d3dFormat;
        optimizedClear.DepthStencil.Depth = optimizedDepth;
        optimizedClear.DepthStencil.Stencil = optimizedStencil;

        ID3D12Resource* tex;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc,
                D3DMappings::toD3DResourceState(initialState), &optimizedClear, IID_PPV_ARGS(&tex))),
            "Failed to create the texture.");
        tex_ = makeComShared(tex);

        resourceDesc_ = tex_->GetDesc();
        format_ = format;
    }

    std::shared_ptr<RenderTarget> Texture::asRenderTarget()
    {
        assert(resourceDesc_.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET && "This texture can not use as the render target.");
        return createRenderDeviceChild<RenderTarget>(getOwnerDevice(), tex_);
    }

    std::shared_ptr<DepthStencil> Texture::asDepthStencil()
    {
        assert(resourceDesc_.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL && "This texture can not use as the depth stencil.");
        return createRenderDeviceChild<DepthStencil>(getOwnerDevice(), tex_);
    }

    ID3D12Resource* Texture::getD3DResource()
    {
        return tex_.get();
    }

    D3D12_SUBRESOURCE_DATA Texture::getD3DSubresource(const void* data) const
    {
        D3D12_SUBRESOURCE_DATA subresource;
        subresource.pData = data;
        subresource.RowPitch = static_cast<LONG_PTR>(resourceDesc_.Width * numBitsOfPixelFormat(format_) / 8);
        subresource.SlicePitch = subresource.RowPitch * resourceDesc_.Height;
        return subresource;
    }

    Texture::Location Texture::locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Format = resourceDesc_.Format;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = 1;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        device->CreateShaderResourceView(tex_.get(), &desc, location);
        return{ location };
    }
    
    Sampler::Sampler()
        : desc_()
    {
        ZeroMemory(&desc_, sizeof(desc_));
        desc_.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        desc_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        desc_.MinLOD = -D3D12_FLOAT32_MAX;
        desc_.MaxLOD = D3D12_FLOAT32_MAX;
        desc_.MipLODBias = 0;
        desc_.MaxAnisotropy = 1;
        desc_.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    }

    Sampler::Location Sampler::locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
    {
        device->CreateSampler(&desc_, location);
        return{ location };
    }
}