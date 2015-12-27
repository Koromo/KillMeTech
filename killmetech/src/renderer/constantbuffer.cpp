#include "constantbuffer.h"
#include "d3dsupport.h"
#include "../core/exception.h"
#include <cstring>

namespace killme
{
    ConstantBuffer::ConstantBuffer(ID3D12Resource* buffer, size_t bufferSize, size_t dataSize)
        : buffer_(makeComUnique(buffer))
        , bufferSize_(bufferSize)
        , mappedData_()
        , dataSize_(dataSize)
    {
        enforce<Direct3DException>(SUCCEEDED(buffer_->Map(0, nullptr, &mappedData_)),
            "Failed to map constant data.");
    }

    void ConstantBuffer::update(const void* src)
    {
        std::memcpy(mappedData_, src, dataSize_);
    }

    D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::getGPUAddress()
    {
        return buffer_->GetGPUVirtualAddress();
    }

    size_t ConstantBuffer::getBufferSize() const
    {
        return bufferSize_;
    }
}