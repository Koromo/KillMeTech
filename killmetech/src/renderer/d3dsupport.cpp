#include "d3dsupport.h"
#include "pixels.h"
#include "../core/exception.h"
#include "../core/platform.h"
#include <Windows.h>
#include <cstring>
#include <cassert>

namespace killme
{
    Direct3DException::Direct3DException(const std::string& msg)
        : Exception(msg)
    {
    }

    D3D12_HEAP_PROPERTIES getD3DDefaultHeapProps()
    {
        D3D12_HEAP_PROPERTIES defaultHeapProps;
        defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        defaultHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        defaultHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        defaultHeapProps.CreationNodeMask = 1;
        defaultHeapProps.VisibleNodeMask = 1;
        return defaultHeapProps;
    }

    D3D12_HEAP_PROPERTIES getD3DUploadHeapProps()
    {
        D3D12_HEAP_PROPERTIES uploadHeapProps;
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        uploadHeapProps.CreationNodeMask = 1;
        uploadHeapProps.VisibleNodeMask = 1;
        return uploadHeapProps;
    }

    D3D12_RESOURCE_DESC describeD3DBuffer(size_t size)
    {
        D3D12_RESOURCE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = size;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        return desc;
    }

    D3D12_RESOURCE_DESC describeD3DTex2D(size_t width, size_t height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
    {
        D3D12_RESOURCE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Alignment = 0;
        desc.Width = width;
        desc.Height = height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = format;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = flags;
        return desc;
    }

    DXGI_FORMAT toD3DPixelFormat(PixelFormat pf)
    {
        switch (pf)
        {
        case PixelFormat::r8g8b8a8_unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;

        default:
            assert(false && "Invalid PixelFormat.");
            return DXGI_FORMAT_UNKNOWN; // For warnnings
        }
    }

    namespace
    {
        // Row unit memcopy
        void memcpySubresource(
            D3D12_MEMCPY_DEST* dest,
            const D3D12_SUBRESOURCE_DATA* src,
            SIZE_T rowSizes,
            UINT numRows,
            UINT numSlices)
        {
            for (UINT z = 0; z < numSlices; ++z)
            {
                BYTE* destSlice = reinterpret_cast<BYTE*>(dest->pData) + dest->SlicePitch * z;
                const BYTE* srcSlice = reinterpret_cast<const BYTE*>(src->pData) + src->SlicePitch * z;
                for (UINT y = 0; y < numRows; ++y)
                {
                    std::memcpy(destSlice + dest->RowPitch * y, srcSlice + src->RowPitch * y, rowSizes);
                }
            }
        }
    }

    void updateSubresources(ID3D12GraphicsCommandList* commands, ID3D12Resource* destResource, ID3D12Resource* intermediate,
        size_t intermediateOffset, size_t firstSubresource, size_t numSubresources, const D3D12_SUBRESOURCE_DATA* srcData)
    {
        const auto memHeapSize = (sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubresources;
        assert(memHeapSize <= SIZE_MAX && "Invalid sumSubresources.");

        const auto memHeap = enforce<Direct3DException>(HeapAlloc(GetProcessHeap(), 0, memHeapSize), "Failed to HeapAlloc().");
        KILLME_SCOPE_EXIT{ HeapFree(GetProcessHeap(), 0, memHeap); };

        const auto layouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(memHeap);
        const auto rowSizes = reinterpret_cast<UINT64*>(layouts + numSubresources);
        const auto numRows = reinterpret_cast<UINT*>(rowSizes + numSubresources);

        const auto destDesc = destResource->GetDesc();

        ID3D12Device* device;
        enforce<Direct3DException>(SUCCEEDED(destResource->GetDevice(IID_PPV_ARGS(&device))), "Failed to GetDevice().");
        KILLME_SCOPE_EXIT{ device->Release(); };

        UINT64 requiredSize;
        device->GetCopyableFootprints(&destDesc, firstSubresource, numSubresources, intermediateOffset, layouts, numRows, rowSizes, &requiredSize);

#ifdef KILLME_DEBUG
        // Check
        D3D12_RESOURCE_DESC intermediateDesc = intermediate->GetDesc();
        if (intermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
            intermediateDesc.Width < requiredSize + layouts[0].Offset ||
            requiredSize > (SIZE_T) - 1 ||
            (destDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
                (firstSubresource != 0 || numSubresources != 1)))
        {
            assert(false && "Failed to updateSubresources().");
        }
#endif

        BYTE* data;
        enforce<Direct3DException>(
            SUCCEEDED(intermediate->Map(0, nullptr, reinterpret_cast<void**>(&data)))
            , "Mapping failed.");

        for (size_t i = 0; i < numSubresources; ++i)
        {
            assert(rowSizes[i] <= (SIZE_T) - 1 && "rowSizes[i] <= (SIZE_T) - 1");
            D3D12_MEMCPY_DEST destData = { data + layouts[i].Offset, layouts[i].Footprint.RowPitch, layouts[i].Footprint.RowPitch * numRows[i] };
            memcpySubresource(&destData, &srcData[i], (SIZE_T)rowSizes[i], numRows[i], layouts[i].Footprint.Depth);
        }

        intermediate->Unmap(0, nullptr);

        if (destDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            //D3D12_BOX srcBox;
            //srcBox.left = static_cast<UINT>(layouts[0].Offset);
            //srcBox.top = 0;
            //srcBox.front = 0;
            //srcBox.right = static_cast<UINT>(layouts[0].Offset + layouts[0].Footprint.Width);
            //srcBox.bottom = 1;
            //srcBox.back = 1;

            commands->CopyBufferRegion(destResource, 0, intermediate, layouts[0].Offset, layouts[0].Footprint.Width);
        }
        else
        {
            for (UINT i = 0; i < numSubresources; ++i)
            {
                D3D12_TEXTURE_COPY_LOCATION dst;
                dst.pResource = destResource;
                dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dst.SubresourceIndex = firstSubresource + i;

                D3D12_TEXTURE_COPY_LOCATION src;
                src.pResource = intermediate;
                src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                src.PlacedFootprint = layouts[i];

                commands->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
            }
        }
    }

    size_t calcRequiredIntermediateSize(ID3D12Resource* dest, size_t intermediateOffset, size_t firstSubresource, size_t numSubresources)
    {
        ID3D12Device* device;
        enforce<Direct3DException>(SUCCEEDED(dest->GetDevice(IID_PPV_ARGS(&device))), "Failed to GetDevice().");
        KILLME_SCOPE_EXIT{ device->Release(); };

        const auto destDesc = dest->GetDesc();

        UINT64 requiredSize;
        device->GetCopyableFootprints(&destDesc, firstSubresource, numSubresources, intermediateOffset, nullptr, nullptr, nullptr, &requiredSize);

        return static_cast<size_t>(requiredSize);
    }
}