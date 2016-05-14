#ifndef _KILLME_GPURESOURCE_H_
#define _KILLME_GPURESOURCE_H_

#include "renderdevice.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    /** Gpu resource state definitions */
    enum class GpuResourceState
    {
        common,
        copyDestination,
        renderTarget,
        vertexBuffer,
        indexBuffer,
        texture,
        present = common /// NOTE: The alias of the common state
    };

    /** GPU resource heap type definitions */
    enum class GpuResourceHeapType
    {
        renderTarget,
        depthStencil,
        buffer,
        sampler
    };

    /** GpuResourceHeap */
    class GpuResourceHeap : public RenderDeviceChild
    {
    private:
        ComUniquePtr<ID3D12DescriptorHeap> heap_;
        D3D12_DESCRIPTOR_HEAP_DESC desc_;

    public:
        /** Initialize */
        void initialize(size_t numResources, GpuResourceHeapType type, bool shaderVisible);

        /** Locate a gpu resource into this heap */
        template <class GpuResource>
        typename GpuResource::Location locate(size_t i, const std::shared_ptr<GpuResource>& resource)
        {
            const auto offset = getD3DOwnerDevice()->GetDescriptorHandleIncrementSize(desc_.Type) * i;
            auto location = heap_->GetCPUDescriptorHandleForHeapStart();
            location.ptr += offset;
            return resource->locate(getD3DOwnerDevice(), location);
        }

        /** Returns the Direct3D descriptor heap */
        ID3D12DescriptorHeap* getD3DHeap();
    };
}

#endif