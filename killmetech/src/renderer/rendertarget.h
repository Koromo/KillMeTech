#ifndef _KILLME_RENDERTARGET_H_
#define _KILLME_RENDERTARGET_H_

#include "renderdevice.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    class Texture;

    /** Render target */
    class RenderTarget : public RenderDeviceChild
    {
    private:
        std::shared_ptr<Texture> tex_;
        D3D12_RESOURCE_DESC desc_;

    public:
        /** Resource location */
        struct Location
        {
            D3D12_CPU_DESCRIPTOR_HANDLE ofD3D;
            DXGI_FORMAT format;
        };

        /** Initialize */
        void initialize(const std::shared_ptr<Texture>& tex);

        /** Return the Direct3D render target */
        ID3D12Resource* getD3DResource();

        /** Create the Direct3D view into a desctipror heap */
        Location locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location);
    };

    /** Create render target interface */
    std::shared_ptr<RenderTarget> renderTargetInterface(const std::shared_ptr<Texture>& tex);
}

#endif