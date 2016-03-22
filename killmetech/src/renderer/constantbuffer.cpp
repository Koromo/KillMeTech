#include "constantbuffer.h"
#include "d3dsupport.h"
#include "../core/exception.h"
#include "../core/math/math.h"
#include <cstring>

namespace killme
{
    ConstantBuffer::~ConstantBuffer()
    {
        buffer_->Unmap(0, nullptr);
    }

    void ConstantBuffer::initialize(size_t size)
    {
        const auto bufferSize = ceiling(size, 256u);
        const auto uploadHeapProps = getD3DUploadHeapProps();
        const auto desc = describeD3DBuffer(bufferSize);

        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the constant buffer.");
        buffer_ = makeComUnique(buffer);

        resourceDesc_ = buffer_->GetDesc();

        enforce<Direct3DException>(
            SUCCEEDED(buffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_))),
            "Failed to map the constant data.");
    }

    void ConstantBuffer::update(const void* src, size_t offset, size_t size)
    {
        std::memcpy(mappedData_ + offset, src, size);
    }

    ConstantBuffer::Location ConstantBuffer::locate(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        desc.BufferLocation = buffer_->GetGPUVirtualAddress();
        desc.SizeInBytes = static_cast<UINT>(resourceDesc_.Width);
        device->CreateConstantBufferView(&desc, location);
        return{ location };
    }
}