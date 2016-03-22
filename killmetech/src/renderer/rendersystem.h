#ifndef _KILLME_RENDERSYSTEM_H_
#define _KILLME_RENDERSYSTEM_H_

#include "rendertarget.h"
#include "depthstencil.h"
#include "../core/optional.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <Windows.h>
#include <array>
#include <memory>

namespace killme
{
    class RenderDevice;
    class GpuResourceHeap;

    struct FrameResource
    {
        std::shared_ptr<RenderTarget> backBuffer;
        std::shared_ptr<DepthStencil> depthStencil;
        RenderTarget::Location backBufferLocation;
        DepthStencil::Location depthStencilLocation;
    };

    /** Render system */
    class RenderSystem
    {
    private:
        static constexpr size_t NUM_BACK_BUFFERS = 2;

        HWND window_;
        std::shared_ptr<RenderDevice> device_;
        ComUniquePtr<IDXGISwapChain3> swapChain_;

        size_t frameIndex_;
        std::shared_ptr<GpuResourceHeap> backBufferHeap_;
        std::array<std::shared_ptr<RenderTarget>, NUM_BACK_BUFFERS> backBuffers_;
        std::array<RenderTarget::Location, NUM_BACK_BUFFERS> backBufferLocations_;

        std::shared_ptr<GpuResourceHeap> depthStencilHeap_;
        std::shared_ptr<DepthStencil> depthStencil_;
        DepthStencil::Location depthStencilLocation_;

    public:
        /** Initialize */
        explicit RenderSystem(HWND window);

        /** Return device */
        std::shared_ptr<RenderDevice> getDevice();

        /** Return the target window */
        HWND getTargetWindow();

        /** Return the current frame resources */
        FrameResource getCurrentFrameResource();

        /** Present the back buffer into the screen */
        void presentBackBuffer();
    };
}

#endif