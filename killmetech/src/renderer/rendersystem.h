#ifndef _KILLME_RENDERSYSTEM_H_
#define _KILLME_RENDERSYSTEM_H_

#include "renderstate.h"
#include "rendertarget.h"
#include "depthstencil.h"
#include "gpuresourceheap.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <Windows.h>
#include <array>
#include <memory>
#include <type_traits>

namespace killme
{
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;
    class Image;
    class Texture;
    class RootSignatureDescription;
    class RootSignature;
    struct PipelineStateDescription;
    class PipelineState;
    class CommandList;

    constexpr size_t NUM_BACK_BUFFERS = 2;

    struct FrameResource
    {
        std::shared_ptr<RenderTarget> backBuffer;
        RenderTarget::View backBufferView;
        DepthStencil::View depthStencilView;
    };

    /** Render system */
    class RenderSystem
    {
    private:
        HWND window_;
        ComUniquePtr<ID3D12Device> device_;
        ComUniquePtr<ID3D12CommandQueue> commandQueue_;
        ComUniquePtr<ID3D12CommandAllocator> commandAllocator_;
        std::shared_ptr<CommandList> commandList_;
        ComUniquePtr<IDXGISwapChain3> swapChain_;

        size_t frameIndex_;
        std::shared_ptr<GpuResourceHeap> backBufferHeap_;
        std::array<std::shared_ptr<RenderTarget>, NUM_BACK_BUFFERS> backBuffers_;
        std::array<RenderTarget::View, NUM_BACK_BUFFERS> backBufferViews_;

        std::shared_ptr<GpuResourceHeap> depthStencilHeap_;
        std::shared_ptr<DepthStencil> depthStencil_;
        DepthStencil::View depthStencilView_;

        ComUniquePtr<ID3D12Fence> fence_;
        std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)> fenceEvent_;
        UINT64 fenceValue_;

    public:
        /** Initialize */
        explicit RenderSystem(HWND window);

        /** Return the target window */
        HWND getTargetWindow();

        /** Return the current frame resources */
        FrameResource getCurrentFrameResource();

		/** Create the vertex buffer */
        std::shared_ptr<VertexBuffer> createVertexBuffer(const void* data, size_t size, size_t stride);

        /** Create the index buffer */
        std::shared_ptr<IndexBuffer> createIndexBuffer(const unsigned short* data, size_t size);

        /** Create the constant buffer */
        std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t size);

        /** Create texture */
        std::shared_ptr<Texture> createTexture(const std::shared_ptr<const Image>& img);

        /** Create the gpu resource heap */
        std::shared_ptr<GpuResourceHeap> createGpuResourceHeap(size_t numResources, GpuResourceHeapType type, GpuResourceHeapFlag flag);

		/** Create the root signature */
        std::shared_ptr<RootSignature> createRootSignature(RootSignatureDescription& desc);

		/** Create the pileline state */
        std::shared_ptr<PipelineState> createPipelineState(const PipelineStateDescription& pipelineDesc);

        /** Store a resource into a resource heap */
        template <class GpuResource>
        typename GpuResource::View createGpuResourceView(const std::shared_ptr<GpuResourceHeap>& heap, size_t i, const std::shared_ptr<GpuResource>& resource)
        {
            const auto d3dHeap = heap->getD3DHeap();
            const auto heapType = heap->getType();
            const auto offset = device_->GetDescriptorHandleIncrementSize(heapType) * i;

            auto location = d3dHeap->GetCPUDescriptorHandleForHeapStart();
            location.ptr += offset;

            return resource->createD3DView(device_.get(), location);
        }

		/** Reset a command list */
        std::shared_ptr<CommandList> beginCommands(const std::shared_ptr<PipelineState>& pipeline);

        /** Execute a command list */
        void executeCommands(const std::shared_ptr<CommandList>& list);

        /** Present the back buffer into the screen */
        void presentBackBuffer();
    };
}

#endif