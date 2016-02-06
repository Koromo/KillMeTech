#ifndef _KILLME_CONSTANTBUFFER_H_
#define _KILLME_CONSTANTBUFFER_H_

#include "d3dsupport.h"
#include "../core/exception.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <cstring>
#include <cmath>

namespace killme
{
    /// TODO: Whether GPU is read only or not
    /** Constant buffer */
    class ConstantBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_RESOURCE_DESC resourceDesc_;
        char* mappedData_;

    public:
        /** Constructs with a Direct3D buffer and size of data */
        explicit ConstantBuffer(ID3D12Resource* buffer)
            : buffer_(makeComUnique(buffer))
            , resourceDesc_(buffer->GetDesc())
            , mappedData_()
        {
            enforce<Direct3DException>(
                SUCCEEDED(buffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_))),
                "Failed to map constant data.");
        }

        /** Updates buffer data */
        void update(const void* src, size_t offset, size_t size) { std::memcpy(mappedData_ + offset, src, size); }

        /** Creates a Direct3D view to desctipror heap */
        void createD3DView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
            desc.BufferLocation = buffer_->GetGPUVirtualAddress();
            desc.SizeInBytes = static_cast<UINT>(resourceDesc_.Width);
            device->CreateConstantBufferView(&desc, location);
        }
    };
}

#endif
