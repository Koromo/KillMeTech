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
        size_t bufferSize_;
        void* mappedData_;
        size_t dataSize_;

    public:
        /** Construct with Direct3D buffer */
        ConstantBuffer(ID3D12Resource* buffer, size_t bufferSize, size_t dataSize)
            : buffer_(makeComUnique(buffer))
            , bufferSize_(bufferSize)
            , mappedData_()
            , dataSize_(dataSize)
        {
            enforce<Direct3DException>(SUCCEEDED(buffer_->Map(0, nullptr, &mappedData_)),
                "Failed to map constant data.");
        }

        /** Update buffer data */
        void update(const void* src) { std::memcpy(mappedData_, src, dataSize_); }

        /** Returns GPU buffer location */
        D3D12_GPU_VIRTUAL_ADDRESS getGPUAddress() { return buffer_->GetGPUVirtualAddress(); }

        /** Returns buffer size */
        size_t getBufferSize() const { return bufferSize_; }
    };
}

#endif