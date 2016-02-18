#ifndef _KILLME_COMMANDLIST_H_
#define _KILLME_COMMANDLIST_H_

#include "vertexdata.h"
#include "rendertarget.h"
#include "depthstencil.h"
#include "gpuresourceheap.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    class Color;
    class RootSignature;
    struct Viewport;
    struct ScissorRect;
    enum class ResourceState;

    /** The command list */
    class CommandList
    {
    private:
        ComUniquePtr<ID3D12GraphicsCommandList> list_;

    public:
        /** Constructs with a Direct3D command list */
        explicit CommandList(ID3D12GraphicsCommandList* list);

        /** Command of clear a render target */
        void clearRenderTarget(RenderTarget::View view, const Color& c);

        /** Command of clear a depth stencil */
        void clearDepthStencil(DepthStencil::View view, float depth);

        /** Command of set a render target and a depth stencil */
        void setRenderTarget(RenderTarget::View rtView, DepthStencil::View dsView);

        /** Command of set a primitive topology */
        void setPrimitiveTopology(PrimitiveTopology pt);

        /** Command of set vertex buffers */
        template <class Binder>
        void setVertexBuffers(const Binder& binder)
        {
            list_->IASetVertexBuffers(0, binder.numViews, binder.views);
        }

        /** Command of set an index buffer */
        void setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer);

        /** Command of set a root signature */
        void setRootSignature(const std::shared_ptr<RootSignature>& signature);

        /** Changes the currently bound resource heaps */
        template <class Range>
        void setGpuResourceHeaps(const Range& heaps, size_t numHeaps)
        {
            std::vector<ID3D12DescriptorHeap*> d3dHeaps;
            d3dHeaps.reserve(numHeaps);
            for (const auto& heap: heaps)
            {
                d3dHeaps.emplace_back(heap->getD3DHeap());
            }
            list_->SetDescriptorHeaps(numHeaps, d3dHeaps.data());
        }

        /** Sets a resource table */
        void setGpuResourceTable(size_t rootParamIndex, const std::shared_ptr<GpuResourceHeap>& heap);

        /** Command of set a viewport */
        void setViewport(const Viewport& vp);

        /** Command of set a scissor rect */
        void setScissorRect(const ScissorRect& rect);

        /** Command of a resource barrior */
        void resourceBarrior(const std::shared_ptr<RenderTarget>& renderTarget, ResourceState before, ResourceState after);

        /** Ends command recording */
        void endCommands();

        /** Draw call */
        void draw(size_t numVertices);

        /** Draw call by index */
        void drawIndexed(size_t numIndices);

        /** Returns the Direct3D command list */
        ID3D12GraphicsCommandList* getD3DCommandList();
    };
}

#endif