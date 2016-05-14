#ifndef _KILLME_RENDERDEVICE_H_
#define _KILLME_RENDERDEVICE_H_

#include "../windows/winsupport.h"
#include "../core/optional.h"
#include "../core/math/color.h"
#include <d3d12.h>
#include <queue>
#include <vector>
#include <utility>
#include <memory>

namespace killme
{
    class PipelineStateCache;
    class CommandAllocator;
    class CommandList;
    class CommandQueue;
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;
    class Texture;
    class PipelineState;
    class ComputePipelineState;
    class GpuResourceHeap;
    enum class GpuResourceHeapType;
    enum class GpuResourceState;
    struct TextureDescription;

    /** RenderDevice */
    class RenderDevice : public std::enable_shared_from_this<RenderDevice>
    {
    private:
        ComUniquePtr<ID3D12Device> device_;
        std::shared_ptr<PipelineStateCache> pipelineCache_; // Unique
        std::queue<std::shared_ptr<CommandAllocator>> readyAllocators_;
        std::vector<std::shared_ptr<CommandAllocator>> queuedAllocators_;
        std::queue<std::shared_ptr<CommandList>> readyCommands_;
        std::vector<std::shared_ptr<CommandList>> queuedCommands_;
        std::shared_ptr<CommandQueue> commandQueue_;

    public:
        /** Construct */
        explicit RenderDevice(ID3D12Device* device);

        /** Initialize */
        void initialize();

        /** Return Direct3D device */
        ID3D12Device* getD3DDevice();

        /** Return Direct3D pipeline state */
        ID3D12PipelineState* getD3DPipeline(const PipelineState& key);

        /** Return Direct3D compute pipeline state */
        ID3D12PipelineState* getD3DPipeline(const ComputePipelineState& key);

        /** Return Direct3D root signature */
        ID3D12RootSignature* getD3DRootSignature(const PipelineState& key);

        /** Return Direct3D root signature */
        ID3D12RootSignature* getD3DRootSignature(const ComputePipelineState& key);

        /** Return a reusable command allocator */
        std::shared_ptr<CommandAllocator> obtainCommandAllocator();

        /** Add a reusable command allocator */
        /// NOT: You need reset allocator before reuse
        void reuseCommandAllocator(const std::shared_ptr<CommandAllocator>& allocator);

        /** Reuse CommandAllocator that is executiong by CommandQueue now. */
        void reuseCommandAllocatorAfterExecution(const std::shared_ptr<CommandAllocator>& allocator);

        /** Return a reusable command list */
        std::shared_ptr<CommandList> obtainCommandList(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline);

        /** Add a reusable command list */
        /// NOT: You need close list before reuse
        void reuseCommandList(const std::shared_ptr<CommandList>& commands);

        /** Reuse CommandList that is executiong by CommandQueue now. */
        void reuseCommandListAfterExecution(const std::shared_ptr<CommandList>& commands);

        /** Return command queue */
        std::shared_ptr<CommandQueue> getCommandQueue();

        /** Create a vertex buffer */
        std::shared_ptr<VertexBuffer> createVertexBuffer(size_t size, size_t stride, GpuResourceState initialState);

        /** Create an index buffer */
        std::shared_ptr<IndexBuffer> createIndexBuffer(size_t size, GpuResourceState initialState);

        /** Create a constant buffer */
        std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t size);

        /** Create a texture */
        std::shared_ptr<Texture> createTexture(const TextureDescription& desc, GpuResourceState initialState, Optional<Color> optimizedClear = nullopt);
        std::shared_ptr<Texture> createTexture(const TextureDescription& desc, GpuResourceState initialState, float optimizedDepth, unsigned optimizedStencil);

        /** Create a gpu resource heap */
        std::shared_ptr<GpuResourceHeap> createGpuResourceHeap(size_t numResources, GpuResourceHeapType type, bool shaderVisible);

        /** Create a pileline state */
        std::shared_ptr<PipelineState> createPipelineState();
    };

    /** DeviceChild interface provides a method to access to the RenderDevice it was created */
    class RenderDeviceChild
    {
    private:
        std::shared_ptr<RenderDevice> owner_;

    public:
        /** For drived classes */
        virtual ~RenderDeviceChild() = default;

        /** Set owner device */
        void setOwnerDevice(const std::shared_ptr<RenderDevice>& owner);

        /** Return owner device */
        std::shared_ptr<RenderDevice> getOwnerDevice();
        ID3D12Device* getD3DOwnerDevice();
    };

    /** Create a RenderReviceChild object */
    template <class T, class... Args>
    std::shared_ptr<T> createRenderDeviceChild(const std::shared_ptr<RenderDevice>& device, Args&&... args)
    {
        const auto child = std::make_shared<T>();
        child->setOwnerDevice(device);
        child->initialize(std::forward<Args>(args)...);
        return child;
    }
}

#endif