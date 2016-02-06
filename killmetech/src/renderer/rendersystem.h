#ifndef _KILLME_RENDERSYSTEM_H_
#define _KILLME_RENDERSYSTEM_H_

#include "gpuresourceheap.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <Windows.h>
#include <array>
#include <memory>

namespace killme
{
    class RenderTarget;
    class DepthStencil;
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;
    class RootSignatureDescription;
    class RootSignature;
    struct PipelineStateDescription;
    class PipelineState;
    class CommandList;

    constexpr size_t NUM_BACK_BUFFERS = 2;

    /** Render system */
    class RenderSystem
    {
    private:
        HWND window_;
        ComUniquePtr<ID3D12Device> device_;
        ComUniquePtr<ID3D12CommandQueue> commandQueue_;
        ComUniquePtr<ID3D12CommandAllocator> commandAllocator_;
        ComUniquePtr<IDXGISwapChain3> swapChain_;

        size_t frameIndex_;
        std::shared_ptr<GpuResourceHeap> renderTargetHeap_;
        std::array<std::shared_ptr<RenderTarget>, NUM_BACK_BUFFERS> renderTargets_;
        std::shared_ptr<GpuResourceHeap> depthStencilHeap_;
        std::shared_ptr<DepthStencil> depthStencil_;

        ComUniquePtr<ID3D12Fence> fence_;
        HANDLE fenceEvent_;
        UINT64 fenceValue_;

    public:
        /** Initializes */
        void startup(HWND window);

        /** Finalizes */
        void shutdown();

        /** Returns a target window */
        HWND getWindow();

        /** Returns a current back buffer */
        std::shared_ptr<RenderTarget> getCurrentBackBuffer();

        /** Returns a depth stencil */
        std::shared_ptr<DepthStencil> getDepthStencil();

		/** Creates a vertex buffer */
        std::shared_ptr<VertexBuffer> createVertexBuffer(const void* data, size_t size, size_t stride);

        /** Creates a index buffer */
        std::shared_ptr<IndexBuffer> createIndexBuffer(const unsigned short* data, size_t size);

        /** Creates a constant buffer */
        std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t size);

        /** Creates a gpu resource heap */
        std::shared_ptr<GpuResourceHeap> createGpuResourceHeap(size_t numResources, GpuResourceHeapType type, GpuResourceHeapFlag flag);

		/** Creates a root signature */
        std::shared_ptr<RootSignature> createRootSignature(RootSignatureDescription& desc);

		/** Creates a pileline state */
        std::shared_ptr<PipelineState> createPipelineState(const PipelineStateDescription& stateDesc);

        /** Creates a command list */
        std::shared_ptr<CommandList> createCommandList();

        /** Stores a resource to resource heap */
        template <class Resource>
        void storeGpuResource(const std::shared_ptr<GpuResourceHeap>& heap, size_t i, const std::shared_ptr<Resource>& resource)
        {
            const auto d3dHeap = heap->getD3DHeap();
            const auto heapType = heap->getType();
            const auto offset = device_->GetDescriptorHandleIncrementSize(heapType) * i;

            auto location = d3dHeap->GetCPUDescriptorHandleForHeapStart();
            location.ptr += offset;

            resource->createD3DView(device_.get(), location);
        }

		/** Resets a command list */
        void resetCommandList(const std::shared_ptr<CommandList>& list, const std::shared_ptr<PipelineState>& pipelineState);

        /** Executes command list */
        void executeCommandList(const std::shared_ptr<CommandList>& list);

        /** Presents screen */
        void presentBackBuffer();
    };

    extern RenderSystem renderSystem;
}

#endif