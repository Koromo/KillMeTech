#ifndef _KILLME_GPURESOURCEHEAP_H_
#define _KILLME_GPURESOURCEHEAP_H_

#include "../windows/winsupport.h"
#include <d3d12.h>
#include <memory>

namespace killme
{
    /** GPU resource heap type definitions */
    enum class GpuResourceHeapType
    {
        renderTarget,
        depthStencil,
        constantBuffer
    };

    /** GPU resource heap flag definitions */
    enum class GpuResourceHeapFlag
    {
        shaderVisible,
        none,
    };

    /** The GPU resource heap */
    /// NOTE: Resource heap is same to the Descriptor heap of Direct3D
    class GpuResourceHeap
    {
    private:
        ComUniquePtr<ID3D12DescriptorHeap> heap_;
        D3D12_DESCRIPTOR_HEAP_TYPE type_;

    public:
        /** Constructs with a Direct3D descriptor heap */
        explicit GpuResourceHeap(ID3D12DescriptorHeap* heap)
            : heap_(makeComUnique(heap))
            , type_(heap->GetDesc().Type)
        {
        }

        /** Returns the Direct3D descriptor heap type */
        D3D12_DESCRIPTOR_HEAP_TYPE getType() const { return type_; }

        /** Returns the Direct3D descriptor heap */
        ID3D12DescriptorHeap* getD3DHeap() { return heap_.get(); }
    };
}

#endif