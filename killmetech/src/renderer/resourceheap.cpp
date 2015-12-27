#include "resourceheap.h"
#include "constantbuffer.h"

namespace killme
{
    ResourceHeap::ResourceHeap(ID3D12DescriptorHeap* heap, D3D12_DESCRIPTOR_HEAP_TYPE type)
        : heap_(makeComUnique(heap))
        , type_(type)
    {
    }

    D3D12_DESCRIPTOR_HEAP_TYPE ResourceHeap::getType() const
    {
        return type_;
    }

    ID3D12DescriptorHeap* ResourceHeap::getD3DHeap()
    {
        return heap_.get();
    }
}