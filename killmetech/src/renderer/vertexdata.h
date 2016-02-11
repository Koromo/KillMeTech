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

    /** The vertex buffer */
    class VertexBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_VERTEX_BUFFER_VIEW view_;

    public:
        /** Constructs with a vertices */
        VertexBuffer(ID3D12Resource* buffer, size_t size, size_t stride);

        /** Returns the Direct3D view */
        D3D12_VERTEX_BUFFER_VIEW getD3DView();
    };

    /** The index buffer */
    class IndexBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_INDEX_BUFFER_VIEW view_;

    public:
        /** Constructs with an indices */
        IndexBuffer(ID3D12Resource* buffer, size_t size);

        /** Returns the Direct3D view */
        D3D12_INDEX_BUFFER_VIEW getD3DView();

        /** Returns the count of index */
        size_t getNumIndices() const;
    };

    /** Primitive topology definitions */
    enum class PrimitiveTopology
    {
        lineList,
        triangeList
    };

    /** Vertex semantic definitions */
    struct VertexSemantic
    {
        static const std::string position;
        static const std::string color;
        static const std::string normal;
        static const std::string texcoord;
    };

    /** The vertex binder */
    template <class Views>
    struct VertexBinder
    {
        Views views;
        size_t numViews;
    };

    /** The set of vertices */
    class VertexData
    {
    private:
        struct Semantic
        {
            std::string name;
            size_t index;
            std::shared_ptr<VertexBuffer> buffer;
        };

        std::vector<Semantic> vertexBuffers_;
        std::shared_ptr<IndexBuffer> indexBuffer_;

    public:
        /** Adds a vertices */
        void addVertices(const std::string& semanticName, size_t semanticIndex, const std::shared_ptr<VertexBuffer>& vertices);

        /** Sets an indices */
        void setIndices(const std::shared_ptr<IndexBuffer>& indices);

        /** Returns the vertex binder from an input layout */
        VertexBinder<std::vector<D3D12_VERTEX_BUFFER_VIEW>> getBinder(const std::shared_ptr<InputLayout>& layout);

        /** Returns the index buffer */
        std::shared_ptr<IndexBuffer> getIndexBuffer();
    };
}

#endif