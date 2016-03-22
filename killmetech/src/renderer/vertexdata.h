#ifndef _KILLME_VERTEXDATA_H_
#define _KILLME_VERTEXDATA_H_

#include "renderdevice.h"
#include "shaders.h"
#include "d3dsupport.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include "../core/utility.h"
#include <d3d12.h>
#include <vector>
#include <memory>
#include <string>
#include <utility>

namespace killme
{
    /** Vertex buffer */
    class VertexBuffer : public RenderDeviceChild
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_RESOURCE_DESC desc_;
        D3D12_VERTEX_BUFFER_VIEW view_;

    public:
        /** Initialize */
        void initialize(size_t size, size_t stride, GpuResourceState initialState);

        /** Return the Direct3D resource */
        ID3D12Resource* getD3DResource();

        /** Return the Direct3D subresource informations */
        D3D12_SUBRESOURCE_DATA getD3DSubresource(const void* data) const;

        /** Return the Direct3D view */
        D3D12_VERTEX_BUFFER_VIEW getD3DView();
    };

    /** Index buffer */
    class IndexBuffer : public RenderDeviceChild
    {
    private:
        ComUniquePtr<ID3D12Resource> buffer_;
        D3D12_RESOURCE_DESC desc_;
        D3D12_INDEX_BUFFER_VIEW view_;

    public:
        /** Initialize */
        void initialize(size_t size, GpuResourceState initialState);

        /** Return the Direct3D resource */
        ID3D12Resource* getD3DResource();

        /** Return the Direct3D subresource informations */
        D3D12_SUBRESOURCE_DATA getD3DSubresource(const void* data) const;

        /** Return the Direct3D view */
        D3D12_INDEX_BUFFER_VIEW getD3DView();

        /** Return the count of index */
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
        /** Add the vertices */
        void addVertices(const std::string& semanticName, size_t semanticIndex, const std::shared_ptr<VertexBuffer>& vertices);

        /** Set the indices */
        void setIndices(const std::shared_ptr<IndexBuffer>& indices);

        /** Return the vertex views from an input layout */
        auto getD3DVertexViews(const std::shared_ptr<VertexShader>& vs)
            -> decltype(emplaceRange(std::vector<D3D12_VERTEX_BUFFER_VIEW>()))
        {
            // Collect vertex buffer views by the input layout
            const auto inputLayout = vs->getD3DInputLayout();
            std::vector<D3D12_VERTEX_BUFFER_VIEW> views(inputLayout.NumElements);

            for (size_t i = 0; i < inputLayout.NumElements; ++i)
            {
                // Find the right buffer view by semantic
                const auto semanticName = inputLayout.pInputElementDescs[i].SemanticName;
                const auto semanticIndex = inputLayout.pInputElementDescs[i].SemanticIndex;

                bool found = false;
                for (const auto& vertices : vertexBuffers_)
                {
                    if (vertices.name == semanticName && vertices.index == semanticIndex)
                    {
                        views[i] = vertices.buffer->getD3DView();
                        found = true;
                        break;
                    }
                }

                enforce<Direct3DException>(found, "The vertex data has not required semantics for the input layout of argments.");
            }

            return emplaceRange(std::move(views));
        }

        /** Return the index buffer */
        std::shared_ptr<IndexBuffer> getIndexBuffer();
    };
}

#endif