#include "vertexdata.h"

namespace killme
{
    VertexBuffer::VertexBuffer(ID3D12Resource* buffer, size_t stride)
        : buffer_(makeComUnique(buffer))
        , desc_(buffer->GetDesc())
        , view_()
    {
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

    D3D12_RESOURCE_DESC VertexBuffer::describeD3D() const
    {
        return desc_;
    }

    D3D12_VERTEX_BUFFER_VIEW VertexBuffer::getD3DView()
    {
        return view_;
    }

    IndexBuffer::IndexBuffer(ID3D12Resource* buffer)
        : buffer_(makeComUnique(buffer))
        , desc_(buffer->GetDesc())
        , view_()
    {
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

    D3D12_RESOURCE_DESC IndexBuffer::describeD3D() const
    {
        return desc_;
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