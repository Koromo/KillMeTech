#pragma once

#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    class RenderTarget;
    class Color;
    enum class PrimitiveTopology;
    struct VertexBinder;
    class RootSignature;
    struct Viewport;
    struct ScissorRect;
    enum class ResourceState;

    /** Command list */
    class CommandList
    {
    private:
        ComUniquePtr<ID3D12GraphicsCommandList> list_;

    public:
        /** Construct with a D3D command list */
        explicit CommandList(ID3D12GraphicsCommandList* list);

        /** Command of clear render target */
        void clearRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget, const Color& c);

        /** Comand of set render target */
        void setRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget);

        /** Command of set primitive topology */
        void setPrimitiveTopology(PrimitiveTopology pt);

        /** Command of set vertex buffers */
        void setVertexBuffers(const VertexBinder& binder);

        /** Command of set root signature */
        void setRootSignature(const std::shared_ptr<RootSignature>& signature);

        /** Command of set viewport */
        void setViewport(const Viewport& vp);

        /** Command of set scissor rect */
        void setScissorRect(const ScissorRect& rect);

        /** Command of render target transition barrior */
        void resourceBarrior(const std::shared_ptr<RenderTarget>& renderTarget, ResourceState before, ResourceState after);

        /** End command recording */
        void close();

        /** Draw call */
        void draw(size_t numVertices);

        /** Returns Direct3D command list */
        ID3D12GraphicsCommandList* getD3DCommandList();
    };
}