#ifndef _KILLME_VERTEXDATA_H_
#define _KILLME_VERTEXDATA_H_

#include "../windows/winsupport.h"
#include <d3d12.h>
#include <vector>
#include <memory>
#include <string>

namespace killme
{
    class VertexShader;

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

    /** Primitive topology definitions */
    enum class PrimitiveTopology
    {
        triangeList
    };

    /** Vertex binder do bind vertices */
    struct VertexBinder
    {
        std::vector<D3D12_VERTEX_BUFFER_VIEW> views; /// TODO: std::vector
        size_t numViews;
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

    public:
        /** Add vertices */
        void addVertices(const std::string& semanticName, size_t semanticIndex, const std::shared_ptr<VertexBuffer>& vertices);

        /** Returns vertex binder */
        VertexBinder getBinder(const std::shared_ptr<const VertexShader>& shader);
    };
}

#endif