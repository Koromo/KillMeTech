#ifndef _KILLME_RENDERSYSTEM_H_
#define _KILLME_RENDERSYSTEM_H_

#include "renderstate.h"
#include "rendertarget.h"
#include "depthstencil.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <Windows.h>
#include <array>
#include <queue>
#include <memory>
#include <type_traits>

namespace killme
{
    class CommandAllocator;
    class CommandList;
    class CommandQueue;
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;
    //class Image;
    class Texture;
    class RootSignatureDescription;
    class RootSignature;
    struct PipelineStateDescription;
    class PipelineState;
    class GpuResourceHeap;
    enum class GpuResourceHeapType;
    enum class GpuResourceHeapFlag;
    enum class PixelFormat;

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
        ComUniquePtr<IDXGISwapChain3> swapChain_;

        size_t frameIndex_;
        std::shared_ptr<GpuResourceHeap> backBufferHeap_;
        std::array<std::shared_ptr<RenderTarget>, NUM_BACK_BUFFERS> backBuffers_;
        std::array<RenderTarget::View, NUM_BACK_BUFFERS> backBufferViews_;

        std::shared_ptr<GpuResourceHeap> depthStencilHeap_;
        std::shared_ptr<DepthStencil> depthStencil_;
        DepthStencil::View depthStencilView_;

        std::queue<std::shared_ptr<CommandAllocator>> readyAllocators_;
        std::vector<std::shared_ptr<CommandAllocator>> queuedAllocators_;
        std::queue<std::shared_ptr<CommandList>> readyLists_;
        std::vector<std::shared_ptr<CommandList>> queuedLists_;
        std::shared_ptr<CommandQueue> commandQueue_;

    public:
        /** Initialize */
        explicit RenderSystem(HWND window);

        /** Return the target window */
        HWND getTargetWindow();

        /** Return the current frame resources */
        FrameResource getCurrentFrameResource();

		/** Create the vertex buffer */
        std::shared_ptr<VertexBuffer> createVertexBuffer(size_t size, size_t stride);

        /** Create the index buffer */
        std::shared_ptr<IndexBuffer> createIndexBuffer(size_t size);

        /** Create the constant buffer */
        std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t size);

        /** Create the texture */
        std::shared_ptr<Texture> createTexture(size_t width, size_t height, PixelFormat format);

        /** Create the gpu resource heap */
        std::shared_ptr<GpuResourceHeap> createGpuResourceHeap(size_t numResources, GpuResourceHeapType type, GpuResourceHeapFlag flag);

		/** Create the root signature */
        std::shared_ptr<RootSignature> createRootSignature(RootSignatureDescription& desc);

		/** Create the pileline state */
        std::shared_ptr<PipelineState> createPipelineState(const PipelineStateDescription& pipelineDesc);

        /** Return a reusable command allocator */
        std::shared_ptr<CommandAllocator> obtainCommandAllocator();

        /** Add a reusable command allocator */
        /// NOT: You need reset allocator before reuse
        void reuseCommandAllocator(const std::shared_ptr<CommandAllocator>& allocator);

        /** Reuse CommandAllocator that is executiong by CommandQueue. */
        void reuseCommandAllocatorAfterExecution(const std::shared_ptr<CommandAllocator>& allocator);

        /** Return a reusable command list */
        std::shared_ptr<CommandList> obtainCommandList(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline);

        /** Add a reusable command list */
        /// NOT: You need close list before reuse
        void reuseCommandList(const std::shared_ptr<CommandList>& list);

        /** Reuse CommandList that is executiong by CommandQueue. */
        void reuseCommandListAfterExecution(const std::shared_ptr<CommandList>& list);

        /** Return command queue */
        std::shared_ptr<CommandQueue> getCommandQueue();

        /** Present the back buffer into the screen */
        void presentBackBuffer();
    };
}

#endif