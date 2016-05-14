#include "gpuresource.h"
#include "d3dsupport.h"
#include "../windows/winsupport.h"
#include "../core/exception.h"
#include <Windows.h>

namespace killme
{
    void GpuResourceHeap::initialize(size_t numResources, GpuResourceHeapType type, bool shaderVisible)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.NumDescriptors = numResources;
        desc.Type = D3DMappings::toD3DDescriptorHeapType(type);
        desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ID3D12DescriptorHeap* heap;
        enforce<Direct3DException>(
            SUCCEEDED(getD3DOwnerDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))),
            "Failed to create the descripter heap.");
        heap_ = makeComUnique(heap);
        desc_ = heap_->GetDesc();
    }

    ID3D12DescriptorHeap* GpuResourceHeap::getD3DHeap()
    {
        return heap_.get();
    }
}