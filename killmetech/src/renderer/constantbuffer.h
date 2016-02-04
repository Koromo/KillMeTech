#ifndef _KILLME_CONSTANTBUFFER_H_
#define _KILLME_CONSTANTBUFFER_H_

#include "d3dsupport.h"
#include "../core/exception.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <cstring>

namespace killme
{
    /// TODO: GPU is read only?
    /// TODO: Type UINT or size_t
    /** Constant buffer */
    class ConstantBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_RESOURCE_DESC resourceDesc_;
        D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc_;
        void* mappedData_;
        size_t dataSize_;

    public:
        /** Construct with Direct3D buffer */
        ConstantBuffer(ID3D12Resource* buffer, size_t dataSize)
            : buffer_(makeComUnique(buffer))
            , resourceDesc_(buffer->GetDesc())
            , viewDesc_()
            , mappedData_()
            , dataSize_(dataSize)
        {
            viewDesc_.BufferLocation = buffer_->GetGPUVirtualAddress();
            viewDesc_.SizeInBytes = static_cast<UINT>(resourceDesc_.Width);
            enforce<Direct3DException>(SUCCEEDED(buffer_->Map(0, nullptr, &mappedData_)),
                "Failed to map constant data.");
        }

        /** Update buffer data */
        void update(const void* src) { std::memcpy(mappedData_, src, dataSize_); }

        /** Create view to desctiprot heap */
        void createView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location) { device->CreateConstantBufferView(&viewDesc_, location); }
    };
}

#endif
