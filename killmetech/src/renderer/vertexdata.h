#ifndef _KILLME_VERTEXDATA_H_
#define _KILLME_VERTEXDATA_H_

#include "../windows/winsupport.h"
#include <d3d12.h>
#include <vector>
#include <memory>
#include <string>

namespace killme
{
    class InputLayout;

    /** Vertex buffer */
    class VertexBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_VERTEX_BUFFER_VIEW view_;

    public:
        /** Construct with vertices */
        VertexBuffer(ID3D12Resource* buffer, size_t size, size_t stride);

        /** Returns Direct3D view */
        D3D12_VERTEX_BUFFER_VIEW getD3DView();
    };

    /** Index buffer */
    class IndexBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_INDEX_BUFFER_VIEW view_;

    public:
        /** Construct with indices */
        IndexBuffer(ID3D12Resource* buffer, size_t size);

        /** Returns Direct3D view */
        D3D12_INDEX_BUFFER_VIEW getD3DView();

        /** Returns count of indes */
        size_t getNumIndices() const;
    };

    /** Primitive topology definitions */
    enum class PrimitiveTopology
    {
        triangeList
    };

    /** Vertex binder do bind vertices */
    template <class Views>
    struct VertexBinder
    {
        Views views; /// TODO: std::vector
        size_t numViews;
    };

    struct VertexSemantic
    {
        static const std::string position;
        static const std::string color;
        static const std::string normal;
        static const std::string texcoord;
    };

    /** Vertex data is set of vertices */
    class VertexData
    {
    private:
        struct VertexSemantic
        {
            std::string semanticName;
            size_t semanticIndex;
            std::shared_ptr<VertexBuffer> buffer;
        };

        std::vector<VertexSemantic> vertexBuffers_;
        std::shared_ptr<IndexBuffer> indexBuffer_;

    public:
        /** Add vertices */
        void addVertices(const std::string& semanticName, size_t semanticIndex, const std::shared_ptr<VertexBuffer>& vertices);

        /** Set indices */
        void setIndices(const std::shared_ptr<IndexBuffer>& indices);

        /** Returns vertex binder */
        VertexBinder<std::vector<D3D12_VERTEX_BUFFER_VIEW>> getBinder(const std::shared_ptr<InputLayout>& layout);

        /** Returns index buffer */
        std::shared_ptr<IndexBuffer> getIndexBuffer();
    };
}

#endif