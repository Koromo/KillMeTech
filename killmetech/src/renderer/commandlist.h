#ifndef _KILLME_COMMANDLIST_H_
#define _KILLME_COMMANDLIST_H_

#include "vertexdata.h"
#include "gpuresourceheap.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    class RenderTarget;
    class DepthStencil;
    class Color;
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
        /** Constructs with a Direct3D command list */
        explicit CommandList(ID3D12GraphicsCommandList* list);

        /** Command of clear render target */
        void clearRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget, const Color& c);

        /** Command of clear depth stencil */
        void clearDepthStencil(const std::shared_ptr<DepthStencil>& depthStencil, float depth);

        /** Comand of set render target and depth stencil */
        void setRenderTarget(const std::shared_ptr<RenderTarget>& renderTarget, const std::shared_ptr<DepthStencil>& depthStencil);

        /** Command of set primitive topology */
        void setPrimitiveTopology(PrimitiveTopology pt);

        /** Command of set vertex buffers */
        template <class Views>
        void setVertexBuffers(const VertexBinder<Views>& binder)
        {
            std::vector<D3D12_VERTEX_BUFFER_VIEW> d3dViews(std::cbegin(binder.views), std::cend(binder.views));
            list_->IASetVertexBuffers(0, binder.numViews, d3dViews.data());
        }

        /** Command of set index buffer */
        void setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer);

        /** Command of set root signature */
        void setRootSignature(const std::shared_ptr<RootSignature>& signature);

        /** Changes currently bound resource heaps */
        template <class Range>
        void setGpuResourceHeaps(Range heaps, size_t numHeaps)
        {
            std::vector<ID3D12DescriptorHeap*> d3dHeaps;
            d3dHeaps.reserve(numHeaps);
            for (const auto& heap: heaps)
            {
                d3dHeaps.push_back(heap->getD3DHeap());
            }
            list_->SetDescriptorHeaps(numHeaps, d3dHeaps.data());
        }

        /** Sets resource table */
        void setGpuResourceTable(size_t rootParamIndex, const std::shared_ptr<GpuResourceHeap>& heap);

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

        /** Draw call with index */
        void drawIndexed(size_t numIndices);

        /** Returns Direct3D command list */
        ID3D12GraphicsCommandList* getD3DCommandList();
    };
}

#endif