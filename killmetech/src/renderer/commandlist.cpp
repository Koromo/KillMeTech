#include "commandlist.h"
#include "commandallocator.h"
#include "pipelinestate.h"
#include "vertexdata.h"
#include "rootsignature.h"
#include "renderstate.h"
#include "resourcebarrior.h"
#include "../core/math/color.h"
#include <cassert>

namespace killme
{
    D3D12_RESOURCE_STATES detail::getD3DResourceState(ResourceState state)
    {
        switch (state)
        {
        case ResourceState::present: return D3D12_RESOURCE_STATE_PRESENT;
        case ResourceState::renderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case ResourceState::copyDestination: return D3D12_RESOURCE_STATE_COPY_DEST;
        case ResourceState::vertexBuffer: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        case ResourceState::indexBuffer: return D3D12_RESOURCE_STATE_INDEX_BUFFER;
        case ResourceState::texture: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        default:
            assert(false && "Invalid resource state.");
            return D3D12_RESOURCE_STATE_COMMON; // For warnings
        }
    }

    CommandList::CommandList(ID3D12GraphicsCommandList* list, const std::shared_ptr<CommandAllocator>& allocator)
        : list_(makeComUnique(list))
        , uploaders_()
        , locked_(false)
        , allocator_(allocator)
    {
    }

    void CommandList::clearRenderTarget(RenderTarget::View view, const Color& c)
    {
        const float rgba[] = {c.r, c.g, c.b, c.a};
        list_->ClearRenderTargetView(view.d3dView, rgba, 0, nullptr);
    }

    void CommandList::clearDepthStencil(DepthStencil::View view, float depth)
    {
        list_->ClearDepthStencilView(view.d3dView, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
    }

    void CommandList::setRenderTarget(RenderTarget::View rtView, DepthStencil::View dsView)
    {
        list_->OMSetRenderTargets(1, &rtView.d3dView, TRUE, &dsView.d3dView);
    }

    namespace
    {
        // Convert to Direct3D primitive topology
        D3D12_PRIMITIVE_TOPOLOGY toD3DPrimitiveTopology(PrimitiveTopology pt)
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
    }

    void CommandList::setPrimitiveTopology(PrimitiveTopology pt)
    {
        list_->IASetPrimitiveTopology(toD3DPrimitiveTopology(pt)); /// TODO: ERROR #611 when used LINELIST
    }

    void CommandList::setIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer)
    {
        const auto view = buffer->getD3DView();
        list_->IASetIndexBuffer(&view);
    }

    void CommandList::setRootSignature(const std::shared_ptr<RootSignature>& signature)
    {
        list_->SetGraphicsRootSignature(signature->getD3DRootSignature());
    }

    void CommandList::setGpuResourceTable(size_t rootParamIndex, const std::shared_ptr<GpuResourceHeap>& heap)
    {
        list_->SetGraphicsRootDescriptorTable(rootParamIndex, heap->getD3DHeap()->GetGPUDescriptorHandleForHeapStart());
    }

    namespace
    {
        // Convert to Direct3D viewport
        D3D12_VIEWPORT toD3DViewport(const Viewport& vp)
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
    }

    void CommandList::setViewport(const Viewport& vp)
    {
        const auto d3dVp = toD3DViewport(vp);
        list_->RSSetViewports(1, &d3dVp);
    }

    namespace
    {
        // Convert to Direct3D scissor rect
        D3D12_RECT toD3DScissorRect(const ScissorRect& rect)
        {
            D3D12_RECT d3dRect;
            d3dRect.top = rect.top;
            d3dRect.left = rect.left;
            d3dRect.right = rect.right;
            d3dRect.bottom = rect.bottom;
            return d3dRect;
        }
    }

    void CommandList::setScissorRect(const ScissorRect& rect)
    {
        const auto d3dRect = toD3DScissorRect(rect);
        list_->RSSetScissorRects(1, &d3dRect);
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
        assert(!locked_ && "This COmmandList is locking.");
        const auto d3dAllocator = allocator->getD3DAllocator();
        const auto d3dPipeline = pipeline ? pipeline->getD3DPipelineState() : nullptr;
        enforce<Direct3DException>(
            SUCCEEDED(list_->Reset(d3dAllocator, d3dPipeline)),
            "Failed to reset command list.");
        uploaders_.clear();
    }

    void CommandList::close()
    {
        enforce<Direct3DException>(SUCCEEDED(list_->Close()), "Failed to close the command list.");
    }

    std::shared_ptr<CommandAllocator> CommandList::getAllocator()
    {
        return allocator_;
    }

    void CommandList::lock(bool b)
    {
        locked_ = b;
    }

    bool CommandList::isLocked() const
    {
        return locked_;
    }

    ID3D12GraphicsCommandList* CommandList::getD3DCommandList()
    {
        return list_.get();
    }
}