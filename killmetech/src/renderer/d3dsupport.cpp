#include "d3dsupport.h"
#include "pixels.h"
#include "gpuresource.h"
#include "vertexdata.h"
#include "renderstate.h"
#include "shaders.h"
#include "texture.h"
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

    DXGI_FORMAT D3DMappings::toD3DDxgiFormat(PixelFormat pf)
    {
        switch (pf)
        {
        case PixelFormat::r8g8b8a8_unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::d16_unorm: return DXGI_FORMAT_D16_UNORM;

        default:
            assert(false && "Invalid PixelFormat.");
            return DXGI_FORMAT_UNKNOWN; // For warnnings
        }
    }

    PixelFormat D3DMappings::toPixelFormat(DXGI_FORMAT fmt)
    {
        switch (fmt)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM: return PixelFormat::r8g8b8a8_unorm;
        case DXGI_FORMAT_D16_UNORM: return PixelFormat::d16_unorm;

        default:
            assert(false && "Invalid PixelFormat.");
            return PixelFormat::r8g8b8a8_unorm; // For warnnings
        }
    }

    D3D12_RESOURCE_STATES D3DMappings::toD3DResourceState(GpuResourceState state)
    {
        switch (state)
        {
        case GpuResourceState::common: return D3D12_RESOURCE_STATE_COMMON;
        //case GpuResourceState::present: return D3D12_RESOURCE_STATE_COMMON;
        case GpuResourceState::renderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case GpuResourceState::copyDestination: return D3D12_RESOURCE_STATE_COPY_DEST;
        case GpuResourceState::vertexBuffer: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        case GpuResourceState::indexBuffer: return D3D12_RESOURCE_STATE_INDEX_BUFFER;
        case GpuResourceState::texture: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        default:
            assert(false && "Invalid resource state.");
            return D3D12_RESOURCE_STATE_COMMON; // For warnings
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY D3DMappings::toD3DPrimitiveTopology(PrimitiveTopology pt)
    {
        switch (pt)
        {
        case PrimitiveTopology::lineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::triangeList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        default:
            assert(false && "Invalid primitive topology.");
            return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED; // For warnings
        }
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE D3DMappings::toD3DPrimitiveTopologyType(PrimitiveTopology pt)
    {
        switch (pt)
        {
        case PrimitiveTopology::lineList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case PrimitiveTopology::triangeList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:
            assert(false && "Invalid primitive topology.");
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED; // For warnings
        }
    }

    D3D12_VIEWPORT D3DMappings::toD3DViewport(const Viewport& vp)
    {
        D3D12_VIEWPORT d3dVp;
        d3dVp.Width = vp.width;
        d3dVp.Height = vp.height;
        d3dVp.TopLeftX = vp.topLeftX;
        d3dVp.TopLeftY = vp.topLeftY;
        d3dVp.MinDepth = vp.minDepth;
        d3dVp.MaxDepth = vp.maxDepth;
        return d3dVp;
    }

    D3D12_RECT D3DMappings::toD3DRect(const ScissorRect& rect)
    {
        D3D12_RECT d3dRect;
        d3dRect.top = rect.top;
        d3dRect.left = rect.left;
        d3dRect.right = rect.right;
        d3dRect.bottom = rect.bottom;
        return d3dRect;
    }

    D3D12_DESCRIPTOR_HEAP_TYPE D3DMappings::toD3DDescriptorHeapType(GpuResourceHeapType type)
    {
        switch (type)
        {
        case GpuResourceHeapType::renderTarget: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        case GpuResourceHeapType::depthStencil: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        case GpuResourceHeapType::buffer: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        case GpuResourceHeapType::sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        default: assert(false && "Invalid heap type.");
        }
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // For warnings
    }

    D3D12_BLEND D3DMappings::toD3DBlend(Blend b)
    {
        switch (b)
        {
        case Blend::one: return D3D12_BLEND_ONE;
        case Blend::zero: return D3D12_BLEND_ZERO;
        default:
            assert(false && "Item not found.");
            return D3D12_BLEND_ONE;
        }
    }

    D3D12_BLEND_OP D3DMappings::toD3DBlendOp(BlendOp op)
    {
        switch (op)
        {
        case BlendOp::add: return D3D12_BLEND_OP_ADD;
        case BlendOp::subtract: return D3D12_BLEND_OP_SUBTRACT;
        case BlendOp::min: return D3D12_BLEND_OP_MIN;
        case BlendOp::max: return D3D12_BLEND_OP_MAX;
        default:
            assert(false && "Item not found.");
            return D3D12_BLEND_OP_ADD;
        }
    }

    D3D12_RENDER_TARGET_BLEND_DESC D3DMappings::toD3DBlendState(const BlendState& blend)
    {
        return{
            blend.enable, FALSE,
            toD3DBlend(blend.src), toD3DBlend(blend.dest), toD3DBlendOp(blend.op),
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };
    }

    D3D12_SHADER_VISIBILITY D3DMappings::toD3DShaderVisibility(ShaderType shader)
    {
        switch (shader)
        {
        case ShaderType::vertex: return D3D12_SHADER_VISIBILITY_VERTEX;
        case ShaderType::pixel: return D3D12_SHADER_VISIBILITY_PIXEL;
        case ShaderType::geometry: return D3D12_SHADER_VISIBILITY_GEOMETRY;
        case ShaderType::compute: return D3D12_SHADER_VISIBILITY_ALL;
        default:
            assert(false && "An invalid ShaderType.");
            return D3D12_SHADER_VISIBILITY_ALL; // For warnings
        }
    }

    D3D_SHADER_INPUT_TYPE D3DMappings::toD3DShaderInputType(BoundResourceType type)
    {
        switch (type)
        {
        case BoundResourceType::cbuffer: return D3D_SIT_CBUFFER;
        case BoundResourceType::texture: return D3D_SIT_TEXTURE;
        case BoundResourceType::sampler: return D3D_SIT_SAMPLER;
        case BoundResourceType::bufferRW: return D3D_SIT_UAV_RWTYPED;
        default:
            assert(false && "Item not found.");
            return D3D_SIT_CBUFFER; // For warnings
        }
    }
    
    BoundResourceType D3DMappings::toBoundResourceType(D3D_SHADER_INPUT_TYPE type)
    {
        switch (type)
        {
        case D3D_SIT_CBUFFER: return BoundResourceType::cbuffer;
        case D3D_SIT_TEXTURE: return BoundResourceType::texture;
        case D3D_SIT_SAMPLER: return BoundResourceType::sampler;
        default:
            assert(false && "Item not found.");
            return BoundResourceType::cbuffer; // For warnings
        }
    }
    
    D3D12_RESOURCE_FLAGS D3DMappings::toD3DResourceFlags(TextureFlags flags)
    {
        D3D12_RESOURCE_FLAGS d3dFlags = D3D12_RESOURCE_FLAG_NONE;

        for (auto test : {
            TextureFlags::none,
            TextureFlags::allowRenderTarget,
            TextureFlags::allowDepthStencil })
        {
            if (flags & test)
            {
                switch (test)
                {
                case TextureFlags::none:
                    d3dFlags |= D3D12_RESOURCE_FLAG_NONE;
                    break;

                case TextureFlags::allowRenderTarget:
                    d3dFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                    break;

                case TextureFlags::allowDepthStencil:
                    d3dFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                    break;

                case TextureFlags::allowUnorderedAccess:
                    d3dFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
                    break;

                default:
                    assert(false && "Invalid TextureFalgs.");
                    break;
                }
            }
        }

        return d3dFlags;
    }
}