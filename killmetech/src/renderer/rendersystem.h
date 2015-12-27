#ifndef _RENDERER_RENDERSYSTEM_H_
#define _RENDERER_RENDERSYSTEM_H_

#include "../windows/winsupport.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <Windows.h>
#include <array>
#include <memory>

namespace killme
{
    class RenderTarget;
    class VertexBuffer;
    class ConstantBuffer;
    class ResourceHeap;
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
        ComUniquePtr<ID3D12Device> device_;
        ComUniquePtr<ID3D12CommandQueue> commandQueue_;
        ComUniquePtr<ID3D12CommandAllocator> commandAllocator_;
        ComUniquePtr<IDXGISwapChain3> swapChain_;
        size_t frameIndex_;
        ComUniquePtr<ID3D12DescriptorHeap> rtvHeap_;
        size_t rtvSize_;
        std::array<std::shared_ptr<RenderTarget>, NUM_BACK_BUFFERS> renderTargets_;
        ComUniquePtr<ID3D12Fence> fence_;
        HANDLE fenceEvent_;
        UINT64 fenceValue_;

    public:
        /** Construct with a target window */
        explicit RenderSystem(HWND window);

        /** Returns current back buffer */
        std::shared_ptr<RenderTarget> getCurrentBackBuffer();

		/** Create vertex buffer */
        std::shared_ptr<VertexBuffer> createVertexBuffer(const void* data, size_t size, size_t stride);

        /** Create constant buffer */
        std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t dataSize);

        /** Create resource heap */
        std::shared_ptr<ResourceHeap> createResourceHeap(size_t numResources);

		/** Create root signature */
        std::shared_ptr<RootSignature> createRootSignature(RootSignatureDescription& desc);

		/** Create pileline state */
        std::shared_ptr<PipelineState> createPipelineState(const PipelineStateDescription& stateDesc);

        /** Create command list */
        std::shared_ptr<CommandList> createCommandList();

        /** Store resource to resource heap */
        void storeResource(const std::shared_ptr<ConstantBuffer>& resource, const std::shared_ptr<ResourceHeap>& heap, size_t i);

		/** Start record commands */
        void startCommandRecording();

		/** Reset command list */
        void resetCommandList(const std::shared_ptr<CommandList>& list, const std::shared_ptr<PipelineState>& pipelineState);

        /** Execute command list */
        void executeCommandList(const std::shared_ptr<CommandList>& list);

        /** Present screen */
        void presentBackBuffer();
    };
}

#endif