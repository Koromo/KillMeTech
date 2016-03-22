#ifndef _KILLME_D3DSUPPORT_H_
#define _KILLME_D3DSUPPORT_H_

#include "../core/exception.h"
#include <d3d12.h>
#include <string>

namespace killme
{
    enum class PixelFormat;
    enum class GpuResourceState;
    enum class PrimitiveTopology;
    enum class GpuResourceHeapType;
    enum class Blend;
    enum class BlendOp;
    enum class ShaderType;
    enum class BoundResourceType;
    enum TextureFlags;
    struct Viewport;
    struct ScissorRect;
    struct BlendState;

    /** Exception of Direct3D */
    class Direct3DException : public Exception
    {
    public:
        /** Construct with a message */
        explicit Direct3DException(const std::string& msg);
    };

    /** Return the default heap properties */
    D3D12_HEAP_PROPERTIES getD3DDefaultHeapProps();

    /** Return the upload heap properties */
    D3D12_HEAP_PROPERTIES getD3DUploadHeapProps();

    /** Return the resource description about buffers */
    D3D12_RESOURCE_DESC describeD3DBuffer(size_t size);

    /** Return the resource description about 2D textures */
    D3D12_RESOURCE_DESC describeD3DTex2D(size_t width, size_t height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags);

    /** Update subresources */
    void updateSubresources(ID3D12GraphicsCommandList* commandList, ID3D12Resource* destResource, ID3D12Resource* intermediate,
        size_t intermediateOffset, size_t firstSubresource, size_t numSubresources, const D3D12_SUBRESOURCE_DATA* pSrcData);

    /** Calclate required intermediate size */
    size_t calcRequiredIntermediateSize(ID3D12Resource* dest, size_t intermediateOffset, size_t firstSubresource, size_t numSubresources);

    /** Exchange any types between KillMeTech API and Direct3D API */
    struct D3DMappings
    {
        /** PixelFormat -> DXGI_FORMAT */
        static DXGI_FORMAT toD3DDxgiFormat(PixelFormat pf);

        /** DXGI_FORMAT -> PixelFormat */
        static PixelFormat toPixelFormat(DXGI_FORMAT fmt);

        /** GpuResourceState -> D3D12_RESOURCE_STATES */
        static D3D12_RESOURCE_STATES toD3DResourceState(GpuResourceState state);

        /** PrimitiveTopology -> D3D12_PRIMITIVE_TOPOLOGY */
        static D3D12_PRIMITIVE_TOPOLOGY toD3DPrimitiveTopology(PrimitiveTopology pt);

        /** PrimitiveTopology -> D3D12_PRIMITIVE_TOPOLOGY_TYPE */
        static D3D12_PRIMITIVE_TOPOLOGY_TYPE toD3DPrimitiveTopologyType(PrimitiveTopology pt);

        /** Viewport -> D3D12_VIEWPORT */
        static D3D12_VIEWPORT toD3DViewport(const Viewport& vp);

        /** ScissorRest -> D3D12_REST */
        static D3D12_RECT toD3DRect(const ScissorRect& rect);

        /** GpuResourceHeapType -> D3D12_DESCRIPTOR_HEAP_TYPE */
        static D3D12_DESCRIPTOR_HEAP_TYPE toD3DDescriptorHeapType(GpuResourceHeapType type);

        /** Blend -> D3D12_BLEND */
        static D3D12_BLEND toD3DBlend(Blend b);

        /** BlendOp -> D3D12_BLEND_OP */
        static D3D12_BLEND_OP toD3DBlendOp(BlendOp op);

        /** BlendState -> D3D12_RENDER_TARGET_BLEND_DESC */
        static D3D12_RENDER_TARGET_BLEND_DESC toD3DBlendState(const BlendState& blend);

        /** ShaderType -> D3D12_SHADER_VISIBILITY */
        static D3D12_SHADER_VISIBILITY toD3DShaderVisibility(ShaderType shader);

        /** BoundResourceType -> D3D_SHADER_INPUT_TYPE */
        static D3D_SHADER_INPUT_TYPE toD3DShaderInputType(BoundResourceType type);

        /** D3D_SHADER_INPUT_TYPE -> BoundResourceType */
        static BoundResourceType toBoundResourceType(D3D_SHADER_INPUT_TYPE type);

        /** TextureFlags -> D3D12_RESOURCE_FLAGS */
        static D3D12_RESOURCE_FLAGS toD3DResourceFlags(TextureFlags flags);
    };
}

#endif
