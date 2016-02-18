#include "rendersystem.h"
#include "vertexdata.h"
#include "constantbuffer.h"
#include "rootsignature.h"
#include "pipelinestate.h"
#include "shader.h"
#include "vertexshader.h"
#include "pixelshader.h"
#include "inputlayout.h"
#include "commandlist.h"
#include "d3dsupport.h"
#include "../resources/resource.h"
#include "../core/exception.h"
#include "../core/string.h"
#include <string>
#include <cstring>
#include <cassert>

namespace killme
{
    namespace
    {
        const DXGI_FORMAT DEPTH_STENCIL_FORMAT = DXGI_FORMAT_D16_UNORM;
    }

    RenderSystem::RenderSystem(HWND window)
        : window_(window)
        , device_()
        , commandQueue_()
        , commandAllocator_()
        , swapChain_()
        , frameIndex_()
        , backBufferHeap_()
        , backBuffers_()
        , backBufferViews_()
        , depthStencilHeap_()
        , depthStencil_()
        , depthStencilView_()
        , fence_()
        , fenceEvent_(nullptr, CloseHandle)
        , fenceValue_()
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
        device_ = makeComUnique(device);

        // Create the command queue
        D3D12_COMMAND_QUEUE_DESC queueDesc;
        ZeroMemory(&queueDesc, sizeof(queueDesc));
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ID3D12CommandQueue* commandQueue;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue))),
            "Failed create the command queue.");
        commandQueue_ = makeComUnique(commandQueue);

        // Create the command allocator
        ID3D12CommandAllocator* commandAllocator;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator))),
            "Failed to create the command allocator.");
        commandAllocator_ = makeComUnique(commandAllocator);

        // Create the swap chain
        RECT clientRect;
        GetClientRect(window_, &clientRect);
        const auto clientWidth = clientRect.right - clientRect.left;
        const auto clientHeight = clientRect.bottom - clientRect.top;

        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
        swapChainDesc.BufferCount = NUM_BACK_BUFFERS;
        swapChainDesc.BufferDesc.Width = clientWidth;
        swapChainDesc.BufferDesc.Height = clientHeight;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
            SUCCEEDED(factory->CreateSwapChain(commandQueue_.get(), &swapChainDesc, &swapChain0)),
            "Failed to create the swap chain.");
        KILLME_SCOPE_EXIT{ swapChain0->Release(); };

        IDXGISwapChain3* swapChain3;
        enforce<Direct3DException>(
            SUCCEEDED(swapChain0->QueryInterface(IID_PPV_ARGS(&swapChain3))),
            "Faild to gat IDXGISwapChain3.");
        swapChain_ = makeComUnique(swapChain3);

        frameIndex_ = swapChain_->GetCurrentBackBufferIndex();

        // Create render targets from back buffers
        backBufferHeap_ = createGpuResourceHeap(NUM_BACK_BUFFERS, GpuResourceHeapType::renderTarget, GpuResourceHeapFlag::none);
        for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            ID3D12Resource* backBuffer;
            enforce<Direct3DException>(
                SUCCEEDED(swapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffer))),
                "Failed to get the back buffer.");
            backBuffers_[i] = std::make_shared<RenderTarget>(backBuffer);
            backBufferViews_[i] = createGpuResourceView(backBufferHeap_, i, backBuffers_[i]);
        }

        // Create the depth stencil
        depthStencilHeap_ = createGpuResourceHeap(1, GpuResourceHeapType::depthStencil, GpuResourceHeapFlag::none);

        const auto defaultHeapProps = getD3DDefaultHeapProps();
        const auto depthStencilDesc = describeD3DTex2D(clientWidth, clientHeight, DEPTH_STENCIL_FORMAT, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        D3D12_CLEAR_VALUE dsClearValue;
        dsClearValue.Format = DEPTH_STENCIL_FORMAT;
        dsClearValue.DepthStencil.Depth = 1.0f;
        dsClearValue.DepthStencil.Stencil = 0;

        ID3D12Resource* depthStencil;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &depthStencilDesc,
                D3D12_RESOURCE_STATE_COMMON, &dsClearValue, IID_PPV_ARGS(&depthStencil))),
            "Failed to create the depth stencil.");

        depthStencil_ = std::make_shared<DepthStencil>(depthStencil);
        depthStencilView_ = createGpuResourceView(depthStencilHeap_, 0, depthStencil_);

        // Create the fence
        ID3D12Fence* fence;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))),
            "Failed to create the fence.");
        fence_ = makeComUnique(fence);
        fenceEvent_.reset(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS));
        fenceValue_ = 1;
    }

    HWND RenderSystem::getTargetWindow()
    {
        return window_;
    }

    FrameResource RenderSystem::getCurrentFrameResource()
    {
        FrameResource frame;
        frame.backBuffer = backBuffers_[frameIndex_];
        frame.backBufferView = backBufferViews_[frameIndex_];
        frame.depthStencilView = depthStencilView_;
        return frame;
    }

    std::shared_ptr<VertexBuffer> RenderSystem::createVertexBuffer(const void* data, size_t size, size_t stride)
    {
        assert(stride <= size && "You need satisfy stride <= size.");

        /// TODO: Now, Only use the upload heap. We can use the default heap to store the data.
        // Use the upload heap
        const auto uploadHeapProps = getD3DUploadHeapProps();
        const auto desc = describeD3DBuffer(size);

        // Create the buffer
        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the vertex buffer.");

        // Upload the data into the buffer
        void* mappedData;
        enforce<Direct3DException>(
            SUCCEEDED(buffer->Map(0, nullptr, &mappedData)),
            "Failed to map the vertices memory.");

        std::memcpy(mappedData, data, static_cast<size_t>(desc.Width));
        buffer->Unmap(0, nullptr);

        return std::make_shared<VertexBuffer>(buffer, stride);
    }

    std::shared_ptr<IndexBuffer> RenderSystem::createIndexBuffer(const unsigned short* data, size_t size)
    {
        /// TODO: Now, Only use the upload heap. We can use the default heap to store the data.
        // Use the upload heap
        const auto uploadHeapProps = getD3DUploadHeapProps();
        const auto desc = describeD3DBuffer(size);

        // Create the buffer
        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the index buffer.");

        // Upload the data into the buffer
        void* mappedData;
        enforce<Direct3DException>(
            SUCCEEDED(buffer->Map(0, nullptr, &mappedData)),
            "Failed to map the indices memory.");

        std::memcpy(mappedData, data, static_cast<size_t>(desc.Width));
        buffer->Unmap(0, nullptr);

        return std::make_shared<IndexBuffer>(buffer);
    }

    std::shared_ptr<ConstantBuffer> RenderSystem::createConstantBuffer(size_t size)
    {
        /// TODO: Now, Only use the upload heap. We can use the default heap to store the data.
        // Use the upload heap
        const size_t bufferSize = size + (256 - size);
        const auto uploadHeapProps = getD3DUploadHeapProps();
        const auto desc = describeD3DBuffer(bufferSize);

        // Create the buffer
        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the constant buffer.");

        return std::make_shared<ConstantBuffer>(buffer);
    }

    namespace
    {
        D3D12_DESCRIPTOR_HEAP_TYPE toD3DHeapType(GpuResourceHeapType type)
        {
            switch (type)
            {
            case GpuResourceHeapType::renderTarget: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            case GpuResourceHeapType::depthStencil: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            case GpuResourceHeapType::constantBuffer: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            default: assert(false && "Invalid heap type.");
            }
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // For warnings
        }

        D3D12_DESCRIPTOR_HEAP_FLAGS toD3DHeapFlag(GpuResourceHeapFlag flag)
        {
            switch (flag)
            {
            case GpuResourceHeapFlag::shaderVisible: return D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            case GpuResourceHeapFlag::none: return D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            default: assert(false && "Invalid heap flag.");
            }
            return D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // For warnings
        }
    }

    std::shared_ptr<GpuResourceHeap> RenderSystem::createGpuResourceHeap(size_t numResources, GpuResourceHeapType type, GpuResourceHeapFlag flag)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.NumDescriptors = numResources;
        desc.Type = toD3DHeapType(type);
        desc.Flags = toD3DHeapFlag(flag);

        ID3D12DescriptorHeap* heap;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))),
            "Failed to create the descripter heap.");

        return std::make_shared<GpuResourceHeap>(heap);
    }

    std::shared_ptr<RootSignature> RenderSystem::createRootSignature(RootSignatureDescription& desc)
    {
        const auto d3dDesc = desc.getD3DDescription();

        ID3DBlob* signature;
        ID3DBlob* err = NULL;
        const auto hr = D3D12SerializeRootSignature(&d3dDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &err);
        if (FAILED(hr))
        {
            std::string msg = "Failed to serialize the root signature.";
            if (err)
            {
                msg += "\n";
                msg += static_cast<char*>(err->GetBufferPointer());
                err->Release();
            }
            throw Direct3DException(msg);
        }

        KILLME_SCOPE_EXIT{ signature->Release(); };

        ID3D12RootSignature* rootSignature;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature))),
            "Failed to create the root sigature.");

        return std::make_shared<RootSignature>(rootSignature);
    }

    std::shared_ptr<PipelineState> RenderSystem::createPipelineState(const PipelineStateDescription& pipelineDesc)
    {
        // Define the rasterizer state
        D3D12_RASTERIZER_DESC rasterizerState;
        ZeroMemory(&rasterizerState, sizeof(rasterizerState));
        rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerState.FrontCounterClockwise = FALSE;
        rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerState.DepthClipEnable = TRUE;
        rasterizerState.MultisampleEnable = FALSE;
        rasterizerState.AntialiasedLineEnable = FALSE;
        rasterizerState.ForcedSampleCount = 0;
        rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // Define the blend state
        D3D12_BLEND_DESC blendState;
        ZeroMemory(&blendState, sizeof(blendState));
        blendState.AlphaToCoverageEnable = FALSE;
        blendState.IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRTBlendDesc = {
            FALSE, FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            blendState.RenderTarget[i] = defaultRTBlendDesc;
        }

        // Define the depth stencil state
        D3D12_DEPTH_STENCIL_DESC depthStencilState;
        ZeroMemory(&depthStencilState, sizeof(depthStencilState));
        depthStencilState.DepthEnable = TRUE;
        depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        depthStencilState.StencilEnable = FALSE;
        depthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        depthStencilState.StencilWriteMask  = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        depthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        depthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        depthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        depthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        // Create the Direct3D pipeline state
        D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dDesc;
        ZeroMemory(&d3dDesc, sizeof(d3dDesc));
        d3dDesc.InputLayout = pipelineDesc.vertexShader.access()->getInputLayout()->getD3DLayout();
        d3dDesc.pRootSignature = pipelineDesc.rootSignature->getD3DRootSignature();
        d3dDesc.VS = { pipelineDesc.vertexShader.access()->getByteCode(), pipelineDesc.vertexShader.access()->getByteCodeSize() };
        d3dDesc.PS = { pipelineDesc.pixelShader.access()->getByteCode(), pipelineDesc.pixelShader.access()->getByteCodeSize() };
        d3dDesc.RasterizerState = rasterizerState;
        d3dDesc.BlendState = blendState;
        d3dDesc.DepthStencilState = depthStencilState;
        d3dDesc.DSVFormat = DEPTH_STENCIL_FORMAT;
        d3dDesc.SampleMask = UINT_MAX;
        d3dDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        d3dDesc.NumRenderTargets = 1;
        d3dDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        d3dDesc.SampleDesc.Count = 1;

        ID3D12PipelineState* pipelineState;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateGraphicsPipelineState(&d3dDesc, IID_PPV_ARGS(&pipelineState))),
            "Failed to create the pipeline state.");

        return std::make_shared<PipelineState>(pipelineState, pipelineDesc);
    }

    std::shared_ptr<CommandList> RenderSystem::createCommandList()
    {
        ID3D12GraphicsCommandList* list;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.get(), nullptr, IID_PPV_ARGS(&list))),
            "Failed to create the command list.");
        list->Close();
        return std::make_shared<CommandList>(list);
    }

    void RenderSystem::beginCommands(const std::shared_ptr<CommandList>& list, const std::shared_ptr<PipelineState>& pipeline)
    {
        enforce<Direct3DException>(
            SUCCEEDED(commandAllocator_->Reset()),
            "Failed to reset the command allocator.");

        const auto d3dPipeline = pipeline ? pipeline->getD3DPipelineState() : nullptr;
        enforce<Direct3DException>(
            SUCCEEDED(list->getD3DCommandList()->Reset(commandAllocator_.get(), d3dPipeline)),
            "Faild to reset the command list.");
    }

    void RenderSystem::executeCommands(const std::shared_ptr<CommandList>& list)
    {
        ID3D12CommandList* d3dLists[] = {list->getD3DCommandList()};
        commandQueue_->ExecuteCommandLists(1, d3dLists);

        // Wait for GPU draw finished
        enforce<Direct3DException>(
            SUCCEEDED(commandQueue_->Signal(fence_.get(), fenceValue_)),
            "Failed to signal of draw end.");
        if (fence_->GetCompletedValue() < fenceValue_)
        {
            enforce<Direct3DException>(
                SUCCEEDED(fence_->SetEventOnCompletion(fenceValue_, fenceEvent_.get())),
                "Failed to set the signal event.");
            WaitForSingleObject(fenceEvent_.get(), INFINITE);
        }
        ++fenceValue_;
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