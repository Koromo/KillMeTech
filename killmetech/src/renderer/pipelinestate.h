#ifndef _KILLME_PIPELINESTATE_H_
#define _KILLME_PIPELINESTATE_H_

#include "renderdevice.h"
#include "shaders.h"
#include "gpuresource.h"
#include "rendertarget.h"
#include "depthstencil.h"
#include "renderstate.h"
#include "../resources/resource.h"
#include "../core/optional.h"
#include "../core/utility.h"
#include "../core/math/color.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <array>

namespace killme
{
    class VertexData;
    class IndexBuffer;
    class CommandList;
    class VertexData;
    enum class PixelFormat;
    enum class PrimitiveTopology;
    
    /** ShaderBoundRequire */
    struct ShaderBoundRequire
    {
        /** A bound shader */
        std::weak_ptr<const BasicShader> boundShader;
        BoundResourceType type; /** Resource type */
        ConstantBufferDescription cbuffer; /** If cbuffer type */
        BoundResourceDescription texture; /** If texture type */
        BoundResourceDescription sampler; /** if sampler type */
    };

    /** GpuResourceHeapRequire */
    class GpuResourceHeapRequire
    {
    private:
        std::vector<size_t> rootIndices_;
        GpuResourceHeapType type_;
        std::vector<ShaderBoundRequire> require_;

    public:
        /** Construct */
        GpuResourceHeapRequire(GpuResourceHeapType type, size_t numResources);

        /** Add a root index */
        void addRootIndex(size_t i);

        /** Return to set to root indices */
        auto getRootIndices() const
            -> decltype(constRange(rootIndices_))
        {
            return constRange(rootIndices_);
        }

        /** Return the type */
        GpuResourceHeapType getType() const;

        /** Return the count of resources */
        size_t getNumResources() const;

        /** Return resource description */
        const ShaderBoundRequire& getResource(size_t i) const;
        ShaderBoundRequire& getResource(size_t i);
    };

    namespace detail
    {
        struct BoundShaders
        {
            Resource<VertexShader> vs;
            Resource<PixelShader> ps;
            Resource<GeometryShader> gs;
            long hashVS;
            long hashPS;
            long hashGS;
        };
    }

    /** GpuResourceTable */
    class GpuResourceTable
    {
    private:
        std::vector<std::shared_ptr<GpuResourceHeap>> heapTable_;
        std::vector<ID3D12DescriptorHeap*> d3dHeapUniqueArray_;
        std::vector<ID3D12DescriptorHeap*> d3dHeapTable_;
        std::vector<GpuResourceHeapRequire> require_;
        std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges_;
        std::vector<D3D12_ROOT_PARAMETER> rootParams_;
        D3D12_ROOT_SIGNATURE_DESC rootSignature_;

    public:
        /** Construct */
        explicit GpuResourceTable(const detail::BoundShaders& boundShaders);

        /** Return the count of required GpuResourceHeaps */
        size_t getNumRequiredHeaps() const;

        /** Return required GpuResourceHeap description */
        const GpuResourceHeapRequire& getRequiredHeap(size_t i) const;

        /** Set GpuResourceHeap */
        void set(size_t i, const std::shared_ptr<GpuResourceHeap>& heap);

        /** Return the hash value of the root signature */
        size_t getRootSignatureHash() const;

        /** Apply resource tables to command list */
        void applyResourceTable(ID3D12GraphicsCommandList* commands);

        /** Create Direct3D RootSignature */
        ID3D12RootSignature* createD3DSignature(ID3D12Device* device) const;
    };

    /** Num of max multiple render targets */
    constexpr size_t MAX_RENDER_TARGETS = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;

    /** PipelineState */
    class PipelineState : public RenderDeviceChild
    {
    private:
        detail::BoundShaders boundShaders_;

        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, MAX_RENDER_TARGETS> renderTargets_;
        D3D12_CPU_DESCRIPTOR_HANDLE depthStencil_;

        std::shared_ptr<VertexData> vertices_;
        std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferViews_;
        D3D12_INDEX_BUFFER_VIEW indexBufferView_;

        D3D12_PRIMITIVE_TOPOLOGY primitiveTopology_;

        D3D12_VIEWPORT viewport_;
        D3D12_RECT scissorRect_;

        mutable std::shared_ptr<GpuResourceTable> resourceTable_;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC topLevelDesc_;

    public:
        /** Initialize */
        void initialize();

        /** Set each shaders */
        /// NOTE: When set a new shader, you need recreate GpuResourceTable by calling getGpuResourceTable()
        void setVShader(const Resource<VertexShader>& vs);
        void setPShader(const Resource<PixelShader>& ps);
        void setGShader(const Resource<GeometryShader>& gs);

        /** Return the gpu resource table for current shaders context */
        std::shared_ptr<GpuResourceTable> getGpuResourceTable();

        /** Set a render target */
        void setRenderTarget(size_t i, Optional<RenderTarget::Location> rt);

        /** Set a depth stencil */
        void setDepthStencil(Optional<DepthStencil::Location> ds);

        /** Set vertex buffers */
        void setVertexBuffers(const std::shared_ptr<VertexData>& vertices, bool setIndices = true);

        /** Set a primitive topology */
        void setPrimitiveTopology(PrimitiveTopology pt);

        /** Set a viewport */
        void setViewport(const Viewport& vp);

        /** Set a scissor rect */
        void setScissorRect(const ScissorRect& sr);

        /** Set a blend state */
        void setBlendState(size_t i, const BlendState& blend);

        /** Return the hash key of top level */
        size_t getTopLevelHash() const;

        /** Return the hash key of pixel shader */
        long getPSHash() const;

        /** Return the hash key of vertex shader */
        long getVSHash() const;

        /** Return the hash key of geometry shader */
        long getGSHash() const;

        /** Return the hash key of root signature */
        size_t getRootSignatureHash() const;

        /** Apply pipeline parameters to commands */
        void applyParameters(ID3D12GraphicsCommandList* commands);

        /** Create a Direct3D pileline state */
        ID3D12PipelineState* createD3DPipeline(ID3D12Device* device, ID3D12RootSignature* rootSignature) const;
        ID3D12RootSignature* createD3DSignature(ID3D12Device* device) const;

    private:
        void createGpuResourceTableIf() const;
    };

    /** PipelieState cache */
    class PipelineStateCache
    {
    private:
        struct GSLevelMap { std::unordered_map<long, ComUniquePtr<ID3D12PipelineState>> map; };
        struct VSLevelMap { std::unordered_map<long, GSLevelMap> map; };
        struct PSLevelMap { std::unordered_map<long, VSLevelMap> map; };
        struct RootSigLevelMap { std::unordered_map<size_t, PSLevelMap> map; };
        struct TopLevelMap { std::unordered_map<size_t, RootSigLevelMap> map; };

        TopLevelMap pipelineMap_;
        std::unordered_map<size_t, ComUniquePtr<ID3D12RootSignature>> signatureMap_;

    public:
        ID3D12PipelineState* getPipeline(ID3D12Device* device, const PipelineState& key);
        ID3D12RootSignature* getSignature(ID3D12Device* device, const PipelineState& key);

    private:
        ID3D12PipelineState* create(ID3D12Device* device, const PipelineState& key);
    };
}

#endif