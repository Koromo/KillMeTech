#ifndef _KILLME_COMMANDLIST_H_
#define _KILLME_COMMANDLIST_H_

#include "renderdevice.h"
#include "rendertarget.h"
#include "depthstencil.h"
#include "gpuresource.h"
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

    /** Command list */
    class CommandList : public RenderDeviceChild
    {
    private:
        ComUniquePtr<ID3D12GraphicsCommandList> list_;
        std::vector<ComUniquePtr<ID3D12Resource>> uploaders_;
        std::shared_ptr<CommandAllocator> allocator_;
        bool protected_;

    public:
        /** Initialize */
        void initialize(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline);

        /** Command of clear a render target */
        void clearRenderTarget(RenderTarget::Location location, const Color& c);

        /** Command of clear a depth stencil */
        void clearDepthStencil(DepthStencil::Location location, float depth);

        /** Command of draw call */
        void draw(size_t numVertices);

        /** Command of draw call by index */
        void drawIndexed(size_t numIndices);

        /** Update a gpu resource */
        template <class GpuResource>
        void updateGpuResource(const std::shared_ptr<GpuResource>& dest, const void* data)
        {
            const auto d3dDest = dest->getD3DResource();

            // Create upload heap
            const auto intermediateSize = calcRequiredIntermediateSize(d3dDest, 0, 0, 1);
            const auto uploadDesc = describeD3DBuffer(intermediateSize);
            const auto uploadHeapProps = getD3DUploadHeapProps();

            ID3D12Resource* uploader;
            enforce<Direct3DException>(
                SUCCEEDED(getD3DOwnerDevice()->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploader))),
                "Failed to create the upload heap.");
            uploaders_.emplace_back(makeComUnique(uploader));

            const auto srcData = dest->getD3DSubresource(data);
            updateSubresources(list_.get(), d3dDest, uploader, 0, 0, 1, &srcData);
        }

        /** Command of the transition barrior */
        template <class GpuResource>
        void transitionBarrior(const std::shared_ptr<GpuResource>& resource, GpuResourceState before, GpuResourceState after)
        {
            D3D12_RESOURCE_BARRIER barrier;
            ZeroMemory(&barrier, sizeof(barrier));
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = resource->getD3DResource();
            barrier.Transition.StateBefore = D3DMappings::toD3DResourceState(before);
            barrier.Transition.StateAfter = D3DMappings::toD3DResourceState(after);
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            list_->ResourceBarrier(1, &barrier);
        }

        /** Reset commands */
        void reset(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline);

        /** Ends command recording */
        void close();

        /** Return owner allocator */
        std::shared_ptr<CommandAllocator> getAllocator();

        /** Protect for reset */
        void protect(bool b);

        /** If true, this list is not resetable */
        bool isProtected() const;

        /** Returns the Direct3D command list */
        ID3D12GraphicsCommandList* getD3DCommandList();
    };
}

#endif