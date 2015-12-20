#ifndef _KILLME_VERTEXBUFFER_H_
#define _KILLME_VERTEXBUFFER_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    enum class PrimitiveTopology
    {
        triangeList
    };

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
}

#endif