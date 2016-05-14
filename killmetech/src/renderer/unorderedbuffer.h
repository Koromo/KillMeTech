#ifndef _KILLME_UNORDEREDBUFFER_H_
#define _KILLME_UNORDEREDBUFFER_H_

#include "renderdevice.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    /** Unordered access buffer */
    class UnorderedBuffer : public RenderDeviceChild
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

        /** Create the Direct3D view into a desctipror heap */
        Location locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location);
    };

    /** Create unordered buffer interface */
    std::shared_ptr<UnorderedBuffer> unorderedBufferInterface(const std::shared_ptr<Texture>& tex);
}

#endif