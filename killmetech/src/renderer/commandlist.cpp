#include "commandlist.h"
#include "rendertarget.h"
#include "depthstencil.h"
#include "rootsignature.h"
#include "renderstate.h"
#include "d3dsupport.h"
#include "../core/math/color.h"
#include "../core/exception.h"
#include <cassert>

namespace killme
{
    CommandList::CommandList(ID3D12GraphicsCommandList* list)
        : list_(makeComUnique(list))
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

    namespace
    {
        // Convert to Direct3D resource state
        D3D12_RESOURCE_STATES toD3DResourceState(ResourceState state)
        {
            switch (state)
            {
            case ResourceState::present: return D3D12_RESOURCE_STATE_PRESENT;
            case ResourceState::renderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
            default:
                assert(false && "Invalid resource state.");
                return D3D12_RESOURCE_STATE_COMMON; // For warnings
            }
        }
    }

    void CommandList::resourceBarrior(const std::shared_ptr<RenderTarget>& renderTarget, ResourceState before, ResourceState after)
    {
        D3D12_RESOURCE_BARRIER barrier;
        ZeroMemory(&barrier, sizeof(barrier));
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTarget->getD3DRenderTarget();
        barrier.Transition.StateBefore = toD3DResourceState(before);
        barrier.Transition.StateAfter = toD3DResourceState(after);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        list_->ResourceBarrier(1, &barrier);
    }

    void CommandList::endCommands()
    {
        enforce<Direct3DException>(SUCCEEDED(list_->Close()), "Failed to close the command list.");
    }

    void CommandList::draw(size_t numVertices)
    {
        list_->DrawInstanced(static_cast<UINT>(numVertices), 1, 0, 0);
    }

    void CommandList::drawIndexed(size_t numIndices)
    {
        list_->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
    }

    ID3D12GraphicsCommandList* CommandList::getD3DCommandList()
    {
        return list_.get();
    }
}