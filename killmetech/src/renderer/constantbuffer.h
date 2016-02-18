#ifndef _KILLME_CONSTANTBUFFER_H_
#define _KILLME_CONSTANTBUFFER_H_

#include "d3dsupport.h"
#include "../core/exception.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <cstring>

namespace killme
{
    /// TODO: Whether GPU is read only or not
    /** The constant buffer */
    class ConstantBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_RESOURCE_DESC resourceDesc_;
        char* mappedData_;

    public:
        /** The resource view */
        struct View
        {
            D3D12_CPU_DESCRIPTOR_HANDLE d3dView;
        };

        /** Constructs with a Direct3D buffer */
        explicit ConstantBuffer(ID3D12Resource* buffer)
            : buffer_(makeComUnique(buffer))
            , resourceDesc_(buffer->GetDesc())
            , mappedData_()
        {
            enforce<Direct3DException>(
                SUCCEEDED(buffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_))),
                "Failed to map the constant data.");
        }

        /** Updates the buffer data */
        void update(const void* src, size_t offset, size_t size) { std::memcpy(mappedData_ + offset, src, size); }

        /** Creates the Direct3D view into a desctipror heap */
        View createD3DView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
            desc.BufferLocation = buffer_->GetGPUVirtualAddress();
            desc.SizeInBytes = static_cast<UINT>(resourceDesc_.Width);
            device->CreateConstantBufferView(&desc, location);
            return{ location };
        }
    };
}

#endif
