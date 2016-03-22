#ifndef _KILLME_CONSTANTBUFFER_H_
#define _KILLME_CONSTANTBUFFER_H_

#include "renderdevice.h"
#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /// TODO: Whether GPU is read only or not
    /** Constant buffer */
    class ConstantBuffer : public RenderDeviceChild
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_RESOURCE_DESC resourceDesc_;
        char* mappedData_;

    public:
        /** Resource location */
        struct Location
        {
            D3D12_CPU_DESCRIPTOR_HANDLE ofD3D;
        };

        /** Destruct */
        ~ConstantBuffer();

        /** Initialize */
        void initialize(size_t size);

        /** Update buffer data */
        void update(const void* src, size_t offset, size_t size);

        /** Create the Direct3D view into a desctipror heap */
        Location locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location);
    };
}

#endif
