#ifndef _KILLME_COMMANDLIST_H_
#define _KILLME_COMMANDLIST_H_

#include "rendertarget.h"
#include "depthstencil.h"
#include "gpuresourceheap.h"
#include "d3dsupport.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include <d3d12.h>
#include <memory>
#include <vector>

namespace killme
{
    class CommandAllocator;
    class PipelineState;
    class Color;
    class RootSignature;
    class IndexBuffer;
    struct Viewport;
    struct ScissorRect;
    enum class PrimitiveTopology;
    enum class ResourceState;

    namespace detail
    {
        D3D12_RESOURCE_STATES getD3DResourceState(ResourceState state);
    }

    /** Command list */
    class CommandList
    {
    private:
        ComUniquePtr<ID3D12GraphicsCommandList> list_;
        std::vector<ComUniquePtr<ID3D12Resource>> uploaders_;
        bool locked_;
        std::shared_ptr<CommandAllocator> allocator_;

    public:
        /** Construct with a Direct3D command list */
        CommandList(ID3D12GraphicsCommandList* list, const std::shared_ptr<CommandAllocator>& allocator);

        /** Command of clear a render target */
        void clearRenderTarget(RenderTarget::View view, const Color& c);

        /** Command of clear a depth stencil */
        void clearDepthStencil(DepthStencil::View view, float depth);

        /** Command of set a render target and a depth stencil */
        void setRenderTarget(RenderTarget::View rtView, DepthStencil::View dsView);

        /** Command of set a primitive topology */
        void setPrimitiveTopology(PrimitiveTopology pt);

        /** Command of set vertex buffers */
        template <class Views>
        void setVertexBuffers(const Views& views)
        {
            std::vector<D3D12_VERTEX_BUFFER_VIEW> d3dViews;
            for (const auto& view : views)
            {
                d3dViews.emplace_back(view);
            }
            list_->IASetVertexBuffers(0, d3dViews.size(), d3dViews.data());
        }

        /** Command of set an index buffer */
        void setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer);

        /** Command of set a root signature */
        void setRootSignature(const std::shared_ptr<RootSignature>& signature);

        /** Changes the currently bound resource heaps */
        template <class Heaps>
        void setGpuResourceHeaps(const Heaps& heaps)
        {
            std::vector<ID3D12DescriptorHeap*> d3dHeaps;
            for (const auto& heap: heaps)
            {
                d3dHeaps.emplace_back(heap->getD3DHeap());
            }
            list_->SetDescriptorHeaps(d3dHeaps.size(), d3dHeaps.data());
        }

        /** Sets a resource table */
        void setGpuResourceTable(size_t rootParamIndex, const std::shared_ptr<GpuResourceHeap>& heap);

        /** Command of set a viewport */
        void setViewport(const Viewport& vp);

        /** Command of set a scissor rect */
        void setScissorRect(const ScissorRect& rect);

        /** Command of draw call */
        void draw(size_t numVertices);

        /** Command of draw call by index */
        void drawIndexed(size_t numIndices);

        /** Update a gpu resource */
        template <class GpuResource>
        void updateGpuResource(const std::shared_ptr<GpuResource>& dest, const void* data)
        {
            const auto d3dDest = dest->getD3DResource();

            ID3D12Device* device;
            enforce<Direct3DException>(SUCCEEDED(d3dDest->GetDevice(IID_PPV_ARGS(&device))), "Failed to GetDevice().");
            KILLME_SCOPE_EXIT{ device->Release(); };

            // Create upload heap
            const auto intermediateSize = calcRequiredIntermediateSize(d3dDest, 0, 0, 1);
            const auto uploadDesc = describeD3DBuffer(intermediateSize);
            const auto uploadHeapProps = getD3DUploadHeapProps();

            ID3D12Resource* uploader;
            enforce<Direct3DException>(
                SUCCEEDED(device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploader))),
                "Failed to create the upload heap.");
            uploaders_.emplace_back(makeComUnique(uploader));

            const auto srcData = dest->getD3DSubresource(data);
            updateSubresources(list_.get(), d3dDest, uploader, 0, 0, 1, &srcData);
        }

        /** Command of the transition barrior */
        template <class GpuResource>
        void transitionBarrior(const std::shared_ptr<GpuResource>& resource, ResourceState before, ResourceState after)
        {
            D3D12_RESOURCE_BARRIER barrier;
            ZeroMemory(&barrier, sizeof(barrier));
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = resource->getD3DResource();
            barrier.Transition.StateBefore = detail::getD3DResourceState(before);
            barrier.Transition.StateAfter = detail::getD3DResourceState(after);
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            list_->ResourceBarrier(1, &barrier);
        }

        /** Reset commands */
        void reset(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline);

        /** Ends command recording */
        void close();

        /** Return owner allocator */
        std::shared_ptr<CommandAllocator> getAllocator();

        /** Lock this list */
        void lock(bool b);

        /** Whether this list is locked or not */
        bool isLocked() const;

        /** Returns the Direct3D command list */
        ID3D12GraphicsCommandList* getD3DCommandList();
    };
}

#endif