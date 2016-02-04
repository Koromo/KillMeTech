#ifndef _KILLME_RESOURCEHEAP_H_
#define _KILLME_RESOURCEHEAP_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /** Resource heap type definitions */
    enum class ResourceHeapType
    {
        renderTarget,
        depthStencil,
        constantBuffer
    };

    /** Resource heap flag definitions */
    enum class ResourceHeapFlag
    {
        shaderVisible,
        none,
    };

    /** Resource heap */
    /// NOTE: Resource heap is same to the Descriptor heap of Direct3D
    class ResourceHeap
    {
    private:
        ComUniquePtr<ID3D12DescriptorHeap> heap_;
        D3D12_DESCRIPTOR_HEAP_TYPE type_;

    public:
        /** Constructs with a Direct3D descriptor heap */
        explicit ResourceHeap(ID3D12DescriptorHeap* heap)
            : heap_(makeComUnique(heap))
            , type_(heap->GetDesc().Type)
        {
        }

        /** Returns Direct3D descriptor heap type */
        D3D12_DESCRIPTOR_HEAP_TYPE getType() const { return type_; }

        /** Returns a Direct3D descriptor heap */
        ID3D12DescriptorHeap* getD3DHeap() { return heap_.get(); }
    };
}

#endif