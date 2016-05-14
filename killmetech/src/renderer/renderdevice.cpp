#include "renderdevice.h"
#include "pipelinestate.h"
#include "commandallocator.h"
#include "commandlist.h"
#include "commandqueue.h"
#include "vertexdata.h"
#include "constantbuffer.h"
#include "texture.h"
#include "gpuresource.h"

namespace killme
{
    RenderDevice::RenderDevice(ID3D12Device* device)
        : device_(makeComUnique(device))
        , pipelineCache_(std::make_shared<PipelineStateCache>())
        , readyAllocators_()
        , queuedAllocators_()
        , readyCommands_()
        , queuedCommands_()
        , commandQueue_()
    {
    }

    void RenderDevice::initialize()
    {
        commandQueue_ = createRenderDeviceChild<CommandQueue>(shared_from_this());
    }

    ID3D12Device* RenderDevice::getD3DDevice()
    {
        return device_.get();
    }

    ID3D12PipelineState* RenderDevice::getD3DPipeline(const PipelineState& key)
    {
        return pipelineCache_->getPipeline(device_.get(), key);
    }

    ID3D12PipelineState* RenderDevice::getD3DPipeline(const ComputePipelineState& key)
    {
        return pipelineCache_->getComputePipeline(device_.get(), key);
    }

    ID3D12RootSignature* RenderDevice::getD3DRootSignature(const PipelineState& key)
    {
        return pipelineCache_->getSignature(device_.get(), key);
    }

    ID3D12RootSignature* RenderDevice::getD3DRootSignature(const ComputePipelineState& key)
    {
        return pipelineCache_->getComputeSignature(device_.get(), key);
    }

    std::shared_ptr<CommandAllocator> RenderDevice::obtainCommandAllocator()
    {
        commandQueue_->updateExecutionState();

        auto it = std::cbegin(queuedAllocators_);
        while (it != std::cend(queuedAllocators_))
        {
            if ((*it)->isProtected())
            {
                ++it;
            }
            else
            {
                (*it)->reset();
                readyAllocators_.emplace(*it);
                it = queuedAllocators_.erase(it);
            }
        }

        if (readyAllocators_.empty())
        {
            readyAllocators_.emplace(createRenderDeviceChild<CommandAllocator>(shared_from_this()));
        }

        const auto front = readyAllocators_.front();
        readyAllocators_.pop();

        return front;
    }

    void RenderDevice::reuseCommandAllocator(const std::shared_ptr<CommandAllocator>& allocator)
    {
        readyAllocators_.emplace(allocator);
    }

    void RenderDevice::reuseCommandAllocatorAfterExecution(const std::shared_ptr<CommandAllocator>& allocator)
    {
        queuedAllocators_.emplace_back(allocator);
    }

    std::shared_ptr<CommandList> RenderDevice::obtainCommandList(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline)
    {
        commandQueue_->updateExecutionState();

        auto it = std::cbegin(queuedCommands_);
        while (it != std::cend(queuedCommands_))
        {
            if ((*it)->isProtected())
            {
                ++it;
            }
            else
            {
                readyCommands_.emplace(*it);
                it = queuedCommands_.erase(it);
            }
        }

        if (readyCommands_.empty())
        {
            return createRenderDeviceChild<CommandList>(shared_from_this(), allocator, pipeline);
        }

        const auto front = readyCommands_.front();
        readyCommands_.pop();

        front->reset(allocator, pipeline);
        return front;
    }

    void RenderDevice::reuseCommandList(const std::shared_ptr<CommandList>& commands)
    {
        readyCommands_.emplace(commands);
    }

    void RenderDevice::reuseCommandListAfterExecution(const std::shared_ptr<CommandList>& commands)
    {
        queuedCommands_.emplace_back(commands);
    }

    std::shared_ptr<CommandQueue> RenderDevice::getCommandQueue()
    {
        return commandQueue_;
    }

    void RenderDeviceChild::setOwnerDevice(const std::shared_ptr<RenderDevice>& owner)
    {
        owner_ = owner;
    }

    std::shared_ptr<RenderDevice> RenderDeviceChild::getOwnerDevice()
    {
        return owner_;
    }

    ID3D12Device* RenderDeviceChild::getD3DOwnerDevice()
    {
        return owner_->getD3DDevice();
    }

    std::shared_ptr<VertexBuffer> RenderDevice::createVertexBuffer(size_t size, size_t stride, GpuResourceState initialState)
    {
        return createRenderDeviceChild<VertexBuffer>(shared_from_this(), size, stride, initialState);
    }

    std::shared_ptr<IndexBuffer> RenderDevice::createIndexBuffer(size_t size, GpuResourceState initialState)
    {
        return createRenderDeviceChild<IndexBuffer>(shared_from_this(), size, initialState);
    }

    std::shared_ptr<ConstantBuffer> RenderDevice::createConstantBuffer(size_t size)
    {
        return createRenderDeviceChild<ConstantBuffer>(shared_from_this(), size);
    }

    std::shared_ptr<Texture> RenderDevice::createTexture(const TextureDescription& desc, GpuResourceState initialState, Optional<Color> optimizedClear)
    {
        return createRenderDeviceChild<Texture>(shared_from_this(), desc, initialState, optimizedClear);
    }

    std::shared_ptr<Texture> RenderDevice::createTexture(const TextureDescription& desc, GpuResourceState initialState, float optimizedDepth, unsigned optimizedStencil)
    {
        return createRenderDeviceChild<Texture>(shared_from_this(), desc, initialState, optimizedDepth, optimizedStencil);
    }

    std::shared_ptr<GpuResourceHeap> RenderDevice::createGpuResourceHeap(size_t numResources, GpuResourceHeapType type, bool shaderVisible)
    {
        return createRenderDeviceChild<GpuResourceHeap>(shared_from_this(), numResources, type, shaderVisible);
    }

    std::shared_ptr<PipelineState> RenderDevice::createPipelineState()
    {
        return createRenderDeviceChild<PipelineState>(shared_from_this());
    }
}