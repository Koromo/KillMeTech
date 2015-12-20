#include "vertexbuffer.h"

namespace killme
{
    VertexBuffer::VertexBuffer(ID3D12Resource* buffer, size_t size, size_t stride)
        : buffer_(makeComUnique(buffer))
        , view_()
    {
        view_.BufferLocation = buffer_->GetGPUVirtualAddress();
        view_.SizeInBytes = static_cast<UINT>(size);
        view_.StrideInBytes = static_cast<UINT>(stride);
    }

    D3D12_VERTEX_BUFFER_VIEW VertexBuffer::getD3DView()
    {
        return view_;
    }
}