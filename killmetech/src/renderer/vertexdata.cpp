#include "vertexdata.h"
#include <cassert>

namespace killme
{
    VertexBuffer::VertexBuffer(ID3D12Resource* buffer, size_t stride)
        : buffer_(makeComUnique(buffer))
        , view_()
    {
        view_.BufferLocation = buffer_->GetGPUVirtualAddress();
        view_.SizeInBytes = static_cast<UINT>(buffer->GetDesc().Width);
        view_.StrideInBytes = static_cast<UINT>(stride);
    }

    D3D12_VERTEX_BUFFER_VIEW VertexBuffer::getD3DView()
    {
        return view_;
    }

    IndexBuffer::IndexBuffer(ID3D12Resource* buffer)
        : buffer_(makeComUnique(buffer))
        , view_()
    {
        view_.BufferLocation = buffer_->GetGPUVirtualAddress();
        view_.SizeInBytes = static_cast<UINT>(buffer->GetDesc().Width);
        view_.Format = DXGI_FORMAT_R16_UINT;
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

    VertexBinder VertexData::getBinder(const std::shared_ptr<InputLayout>& layout)
    {
        // Collect vertex buffer views by the input layout
        const auto d3dLayout = layout->getD3DLayout();

        VertexBinder binder;
        binder.viewsArray.resize(d3dLayout.NumElements);
        binder.numViews = d3dLayout.NumElements;

        for (size_t i = 0; i < d3dLayout.NumElements; ++i)
        {
            // Find the right buffer view by semantic
            const auto semanticName = d3dLayout.pInputElementDescs[i].SemanticName;
            const auto semanticIndex = d3dLayout.pInputElementDescs[i].SemanticIndex;

            bool found = false;
            for (const auto& vertices : vertexBuffers_)
            {
                if (vertices.name == semanticName && vertices.index == semanticIndex)
                {
                    binder.viewsArray[i] = vertices.buffer->getD3DView();
                    found = true;
                    break;
                }
            }

            assert(found && "The vertex data is not usable for the input layout of argments.");
        }

        binder.views = binder.viewsArray.data();
        return binder;

    }
    std::shared_ptr<IndexBuffer> VertexData::getIndexBuffer()
    {
        return indexBuffer_;
    }
}