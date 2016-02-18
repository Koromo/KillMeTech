#ifndef _KILLME_VERTEXDATA_H_
#define _KILLME_VERTEXDATA_H_

#include "inputlayout.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <vector>
#include <memory>
#include <string>
#include <utility>

namespace killme
{
    /** The vertex buffer */
    class VertexBuffer
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_VERTEX_BUFFER_VIEW view_;

    public:
        /** Constructs with a vertices */
        VertexBuffer(ID3D12Resource* buffer, size_t stride);

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
        explicit IndexBuffer(ID3D12Resource* buffer);

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
    struct SemanticNames
    {
        static const std::string position;
        static const std::string color;
        static const std::string normal;
        static const std::string texcoord;
    };

    /** The vertex binder */
    struct VertexBinder
    {
        std::vector<D3D12_VERTEX_BUFFER_VIEW> viewsArray;
        const D3D12_VERTEX_BUFFER_VIEW* views;
        size_t numViews;
    };

    /** The set of vertices */
    class VertexData
    {
    private:
        struct VBuffer
        {
            std::string name;
            size_t index;
            std::shared_ptr<VertexBuffer> buffer;
        };

        std::vector<VBuffer> vertexBuffers_;
        std::shared_ptr<IndexBuffer> indexBuffer_;

    public:
        /** Adds a vertices */
        void addVertices(const std::string& semanticName, size_t semanticIndex, const std::shared_ptr<VertexBuffer>& vertices);

        /** Sets an indices */
        void setIndices(const std::shared_ptr<IndexBuffer>& indices);

        /** Returns the vertex binder from an input layout */
        VertexBinder getBinder(const std::shared_ptr<InputLayout>& layout);

        /** Returns the index buffer */
        std::shared_ptr<IndexBuffer> getIndexBuffer();
    };
}

#endif