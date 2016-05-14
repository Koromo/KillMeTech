#include "rendersystem.h"
#include "gpuresource.h"
#include "texture.h"
#include "renderdevice.h"
#include "pixels.h"
#include "commandqueue.h"
#include "d3dsupport.h"
#include "../core/exception.h"

namespace killme
{
    RenderSystem::RenderSystem(HWND window)
        : window_(window)
        , device_()
        , swapChain_()
        , frameIndex_()
        , backBufferHeap_()
        , backBuffers_()
        , backBufferLocations_()
        , depthStencilHeap_()
        , depthStencil_()
        , depthStencilLocation_()
    {
        // Enable the debug layer
#ifdef _DEBUG
        ID3D12Debug* debugController;
        enforce<Direct3DException>(
            SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))),
            "Failed to ebable the debug layer.");
        debugController->EnableDebugLayer();
        KILLME_SCOPE_EXIT{ debugController->Release(); };
#endif

        // Create the device
        ID3D12Device* device;
        enforce<Direct3DException>(
            SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))),
            "Failed to create the device.");
        device_ = std::make_shared<RenderDevice>(device);
        device_->initialize();

        // Create the swap chain
        RECT clientRect;
        GetClientRect(window_, &clientRect);
        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        const auto RENDER_TARGET_FORMAT = PixelFormat::r8g8b8a8_unorm;

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = NUM_BACK_BUFFERS;
        swapChainDesc.BufferDesc.Width = clientWidth;
        swapChainDesc.BufferDesc.Height = clientHeight;
        swapChainDesc.BufferDesc.Format = D3DMappings::toD3DDxgiFormat(RENDER_TARGET_FORMAT);
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.OutputWindow = window_;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;

        IDXGIFactory4* factory;
        enforce<Direct3DException>(
            SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))),
            "Failed to create the DXGI factory.");
        KILLME_SCOPE_EXIT{ factory->Release(); };

        IDXGISwapChain* swapChain0;
        enforce<Direct3DException>(
            SUCCEEDED(factory->CreateSwapChain(device_->getCommandQueue()->getD3DCommandQueue(), &swapChainDesc, &swapChain0)),
            "Failed to create the swap chain.");
        KILLME_SCOPE_EXIT{ swapChain0->Release(); };

        IDXGISwapChain3* swapChain3;
        enforce<Direct3DException>(
            SUCCEEDED(swapChain0->QueryInterface(IID_PPV_ARGS(&swapChain3))),
            "Faild to gat IDXGISwapChain3.");
        swapChain_ = makeComUnique(swapChain3);

        frameIndex_ = swapChain_->GetCurrentBackBufferIndex();

        // Create render targets from back buffers
        backBufferHeap_ = device_->createGpuResourceHeap(NUM_BACK_BUFFERS, GpuResourceHeapType::renderTarget, false);
        for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            ID3D12Resource* backBuffer;
            enforce<Direct3DException>(
                SUCCEEDED(swapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffer))),
                "Failed to get the back buffer.");

            const auto tex = createRenderDeviceChild<Texture>(device_, backBuffer);
            backBuffers_[i] = renderTargetInterface(tex);
            backBufferLocations_[i] = backBufferHeap_->locate(i, backBuffers_[i]);
        }

        // Create the depth stencil
        depthStencilHeap_ = device_->createGpuResourceHeap(1, GpuResourceHeapType::depthStencil, false);

        TextureDescription dsDesc;
        dsDesc.width = clientWidth;
        dsDesc.height = clientHeight;
        dsDesc.format = PixelFormat::d16_unorm;
        dsDesc.flags = TextureFlags::allowDepthStencil;
        const auto depthStencilTexture = device_->createTexture(dsDesc, GpuResourceState::common, 1, 0);
        depthStencil_ = depthStencilInterface(depthStencilTexture);
        depthStencilLocation_ = depthStencilHeap_->locate(0, depthStencil_);
    }

    std::shared_ptr<RenderDevice> RenderSystem::getDevice()
    {
        return device_;
    }

    HWND RenderSystem::getTargetWindow()
    {
        return window_;
    }

    FrameResource RenderSystem::getCurrentFrameResource()
    {
        FrameResource frame;
        frame.backBuffer = backBuffers_[frameIndex_];
        frame.depthStencil = depthStencil_;
        frame.backBufferLocation = backBufferLocations_[frameIndex_];
        frame.depthStencilLocation = depthStencilLocation_;
        return frame;
    }

    void RenderSystem::presentBackBuffer()
    {
        // Flip screen
        enforce<Direct3DException>(
            SUCCEEDED(swapChain_->Present(1, 0)),
            "Failed to present the back buffer.");

        // Update frame index
        frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
    }
}