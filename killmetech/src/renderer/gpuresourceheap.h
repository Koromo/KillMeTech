#ifndef _KILLME_GPURESOURCEHEAP_H_
#define _KILLME_GPURESOURCEHEAP_H_

#include "d3dsupport.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    /** GPU resource heap type definitions */
    enum class GpuResourceHeapType
    {
        rtv,
        dsv,
        cbv_srv,
        sampler
    };

    /** GPU resource heap flag definitions */
    enum class GpuResourceHeapFlag
    {
        shaderVisible,
        none,
    };

    /** GPU resource heap */
    /// NOTE: Resource heap is same to the Descriptor heap of Direct3D
    class GpuResourceHeap
    {
    private:
        ComUniquePtr<ID3D12DescriptorHeap> heap_;
        D3D12_DESCRIPTOR_HEAP_DESC desc_;

    public:
        /** Construct with a Direct3D descriptor heap */
        explicit GpuResourceHeap(ID3D12DescriptorHeap* heap)
            : heap_(makeComUnique(heap))
            , desc_(heap->GetDesc())
        {
        }

        /** Store a gpu resource into this heap */
        template <class GpuResource>
        typename GpuResource::View createView(size_t i, const std::shared_ptr<GpuResource>& resource)
        {
            ID3D12Device* device;
            enforce<Direct3DException>(SUCCEEDED(heap_->GetDevice(IID_PPV_ARGS(&device))), "Failed to GetDevice().");
            KILLME_SCOPE_EXIT{ device->Release(); };

            const auto offset = device->GetDescriptorHandleIncrementSize(desc_.Type) * i;

            auto location = heap_->GetCPUDescriptorHandleForHeapStart();
            location.ptr += offset;

            return resource->createD3DView(device, location);
        }

        /** Returns the Direct3D descriptor heap */
        ID3D12DescriptorHeap* getD3DHeap() { return heap_.get(); }
    };
}

#endif