#include "commandlist.h"
#include "pipelinestate.h"
#include "../core/math/color.h"

namespace killme
{
    void CommandList::initialize(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline)
    {
        initializeImpl(allocator, pipeline);
        if (pipeline)
        {
            list_->SetGraphicsRootSignature(getOwnerDevice()->getD3DRootSignature(*pipeline));
            pipeline->applyParameters(list_.get());
        }
    }

    void CommandList::initialize(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<ComputePipelineState>& pipeline)
    {
        initializeImpl(allocator, pipeline);
        if (pipeline)
        {
            list_->SetComputeRootSignature(getOwnerDevice()->getD3DRootSignature(*pipeline));
            pipeline->applyParameters(list_.get());
        }
    }

    void CommandList::clearRenderTarget(RenderTarget::Location location, const Color& c)
    {
        const float rgba[] = {c.r, c.g, c.b, c.a};
        list_->ClearRenderTargetView(location.ofD3D, rgba, 0, nullptr);
    }

    void CommandList::clearDepthStencil(DepthStencil::Location location, float depth)
    {
        list_->ClearDepthStencilView(location.ofD3D, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
    }

    void CommandList::draw(size_t numVertices)
    {
        list_->DrawInstanced(static_cast<UINT>(numVertices), 1, 0, 0);
    }

    void CommandList::drawIndexed(size_t numIndices)
    {
        list_->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
    }

    void CommandList::reset(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline)
    {
        resetImpl(allocator, pipeline);
        if (pipeline)
        {
            list_->SetGraphicsRootSignature(getOwnerDevice()->getD3DRootSignature(*pipeline));
            pipeline->applyParameters(list_.get());
        }
    }

    void CommandList::reset(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<ComputePipelineState>& pipeline)
    {
        resetImpl(allocator, pipeline);
        if (pipeline)
        {
            list_->SetComputeRootSignature(getOwnerDevice()->getD3DRootSignature(*pipeline));
            pipeline->applyParameters(list_.get());
        }
    }

    void CommandList::close()
    {
        enforce<Direct3DException>(SUCCEEDED(list_->Close()), "Failed to close the command list.");
    }

    std::shared_ptr<CommandAllocator> CommandList::getAllocator()
    {
        return allocator_;
    }

    void CommandList::protect(bool b)
    {
        protected_ = b;
    }

    bool CommandList::isProtected() const
    {
        return protected_;
    }

    ID3D12GraphicsCommandList* CommandList::getD3DCommandList()
    {
        return list_.get();
    }
}