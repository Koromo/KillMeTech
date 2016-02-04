#include "vertexdata.h"
#include "inputlayout.h"
#include <cassert>

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

    IndexBuffer::IndexBuffer(ID3D12Resource* buffer, size_t size)
        : buffer_(makeComUnique(buffer))
        , view_()
    {
        view_.BufferLocation = buffer_->GetGPUVirtualAddress();
        view_.SizeInBytes = static_cast<UINT>(size);
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

    const std::string VertexSemantic::position = "POSITION";
    const std::string VertexSemantic::color = "COLOR";
    const std::string VertexSemantic::normal = "NORMAL";
    const std::string VertexSemantic::texcoord = "TEXCOORD";

    void VertexData::addVertices(const std::string& semanticName, size_t semanticIndex, const std::shared_ptr<VertexBuffer>& vertices)
    {
        vertexBuffers_.push_back({semanticName, semanticIndex, vertices});
    }

    void VertexData::setIndices(const std::shared_ptr<IndexBuffer>& indices)
    {
        indexBuffer_ = indices;
    }

    VertexBinder<std::vector<D3D12_VERTEX_BUFFER_VIEW>> VertexData::getBinder(const std::shared_ptr<InputLayout>& layout)
    {
        // Collect vertex buffer views by input layout
        const auto d3dLayout = layout->getD3DLayout();

        VertexBinder<std::vector<D3D12_VERTEX_BUFFER_VIEW>> binder;
        binder.numViews = d3dLayout.NumElements;
        binder.views.resize(d3dLayout.NumElements);

        for (size_t i = 0; i < d3dLayout.NumElements; ++i)
        {
            // Find right buffer view by semantic
            const auto semanticName = d3dLayout.pInputElementDescs[i].SemanticName;
            const auto semanticIndex = d3dLayout.pInputElementDescs[i].SemanticIndex;

            bool found = false;
            for (const auto& vertices : vertexBuffers_)
            {
                if (vertices.semanticName == semanticName && vertices.semanticIndex == semanticIndex)
                {
                    binder.views[i] = vertices.buffer->getD3DView();
                    found = true;
                    break;
                }
            }

            assert(found && "Vertex data layout and shader input layout is not match.");
        }

        return binder;
    }

    std::shared_ptr<IndexBuffer> VertexData::getIndexBuffer()
    {
        return indexBuffer_;
    }
}