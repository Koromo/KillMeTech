#ifndef _KILLME_TEXTURE_H_
#define _KILLME_TEXTURE_H_

#include "renderdevice.h"
#include "pixels.h"
#include "../resources/resource.h"
#include "../windows/winsupport.h"
#include "../core/optional.h"
#include "../core/math/color.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    class RenderTarget;
    class DepthStencil;
    enum class GpuResourceState;

    /** Texture bindable flags */
    enum TextureFlags
    {
        none = 0,
        allowRenderTarget = 1 << 0,
        allowDepthStencil = 1 << 1
    };

    /** Texture resource */
    class Texture : public RenderDeviceChild, public IsResource
    {
    private:
        ComSharedPtr<ID3D12Resource> tex_;
        D3D12_RESOURCE_DESC resourceDesc_;
        PixelFormat format_;

    public:
        /** Resource location */
        struct Location
        {
            D3D12_CPU_DESCRIPTOR_HANDLE ofD3D;
        };

        /** Initialize */
        void initialize(size_t width, size_t height, PixelFormat format, TextureFlags flags,
            GpuResourceState initialState, Optional<Color> optimizedClear);
        void initialize(size_t width, size_t height, PixelFormat format, TextureFlags flags,
            GpuResourceState initialState, float optimizedDepth, unsigned optimizedStencil);

        /** Return the render target interface */
        std::shared_ptr<RenderTarget> asRenderTarget();

        /** Return the depth stencil interface */
        std::shared_ptr<DepthStencil> asDepthStencil();

        /** Return the Direct3D resource */
        ID3D12Resource* getD3DResource();

        /** Return the Direct3D subresource informations */
        D3D12_SUBRESOURCE_DATA getD3DSubresource(const void* data) const;

        /** Create the Direct3D view into a desctipror heap */
        Location locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location);
    };

    /** Sampler */
    class Sampler
    {
    private:
        D3D12_SAMPLER_DESC desc_;

    public:
        /** Resource location */
        struct Location
        {
            D3D12_CPU_DESCRIPTOR_HANDLE ofD3D;
        };

        /** Construct */
        Sampler();

        /** Create the Direct3D view into a desctipror heap */
        Location locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location);
    };
}

#endif