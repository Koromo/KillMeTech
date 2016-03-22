#include "vertexdata.h"
#include <cassert>

namespace killme
{
    void VertexBuffer::initialize(size_t size, size_t stride, GpuResourceState initialState)
    {
        assert(stride <= size && "You need satisfy stride <= size.");

        const auto defaultHeapProps = getD3DDefaultHeapProps();
        const auto desc = describeD3DBuffer(size);

        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc,
                D3DMappings::toD3DResourceState(initialState), nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the vertex buffer.");
        buffer_ = makeComUnique(buffer);

        desc_ = buffer_->GetDesc();
        view_.BufferLocation = buffer_->GetGPUVirtualAddress();
        view_.SizeInBytes = static_cast<UINT>(desc_.Width);
        view_.StrideInBytes = static_cast<UINT>(stride);
    }

    ID3D12Resource* VertexBuffer::getD3DResource()
    {
        return buffer_.get();
    }

    D3D12_SUBRESOURCE_DATA VertexBuffer::getD3DSubresource(const void* data) const
    {
        D3D12_SUBRESOURCE_DATA subresource;
        subresource.pData = data;
        subresource.RowPitch = static_cast<LONG_PTR>(desc_.Width);
        subresource.SlicePitch = subresource.RowPitch;
        return subresource;
    }

    D3D12_VERTEX_BUFFER_VIEW VertexBuffer::getD3DView()
    {
        return view_;
    }

    void IndexBuffer::initialize(size_t size, GpuResourceState initialState)
    {
        const auto defaultHeapProps = getD3DDefaultHeapProps();
        const auto desc = describeD3DBuffer(size);

        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc,
                D3DMappings::toD3DResourceState(initialState), nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the index buffer.");
        buffer_ = makeComUnique(buffer);

        desc_ = buffer_->GetDesc();
        view_.BufferLocation = buffer_->GetGPUVirtualAddress();
        view_.SizeInBytes = static_cast<UINT>(buffer->GetDesc().Width);
        view_.Format = DXGI_FORMAT_R16_UINT;
    }

    ID3D12Resource* IndexBuffer::getD3DResource()
    {
        return buffer_.get();
    }

    D3D12_SUBRESOURCE_DATA IndexBuffer::getD3DSubresource(const void* data) const
    {
        D3D12_SUBRESOURCE_DATA subresource;
        subresource.pData = data;
        subresource.RowPitch = static_cast<LONG_PTR>(desc_.Width);
        subresource.SlicePitch = subresource.RowPitch;
        return subresource;
    }

    D3D12_INDEX_BUFFER_VIEW IndexBuffer::getD3DView()
    {
        return view_;
    }

    size_t IndexBuffer::getNumIndices() const
    {
        return view_.SizeInBytes / sizeof(unsigned short);
    }

    const std::string SemanticNames::position   = "POSITION";
    const std::string SemanticNames::color      = "COLOR";
    const std::string SemanticNames::normal     = "NORMAL";
    const std::string SemanticNames::texcoord   = "TEXCOORD";

    void VertexData::addVertices(const std::string& semanticName, size_t semanticIndex, const std::shared_ptr<VertexBuffer>& vertices)
    {
        vertexBuffers_.emplace_back(VBuffer{ semanticName, semanticIndex, vertices });
    }

    void VertexData::setIndices(const std::shared_ptr<IndexBuffer>& indices)
    {
        indexBuffer_ = indices;
    }

    std::shared_ptr<IndexBuffer> VertexData::getIndexBuffer()
    {
        return indexBuffer_;
    }
}