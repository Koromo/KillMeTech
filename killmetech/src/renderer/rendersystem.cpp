#include "rendersystem.h"
#include "rendertarget.h"
#include "vertexdata.h"
#include "constantbuffer.h"
#include "resourceheap.h"
#include "rootsignature.h"
#include "pipelinestate.h"
#include "vertexshader.h"
#include "pixelshader.h"
#include "commandlist.h"
#include "d3dsupport.h"
#include "../core/exception.h"
#include <cstring>
#include <cassert>

namespace killme
{
    RenderSystem::RenderSystem(HWND window)
        : device_()
        , commandQueue_()
        , commandAllocator_()
        , swapChain_()
        , frameIndex_()
        , rtvHeap_()
        , rtvSize_()
        , renderTargets_()
        , fence_()
        , fenceEvent_()
        , fenceValue_()
    {
        // Enable the debug layer
#ifdef _DEBUG
        ID3D12Debug* debugController;
        enforce<Direct3DException>(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))),
            "Failed to ebable the debug layer.");
        const auto debugControllerHolder = makeComUnique(debugController);
        debugController->EnableDebugLayer();
#endif

        // Create the device
        ID3D12Device* device;
        enforce<Direct3DException>(SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))),
            "Failed to create device.");
        device_ = makeComUnique(device);

        // Create the command queue
        D3D12_COMMAND_QUEUE_DESC queueDesc;
        ZeroMemory(&queueDesc, sizeof(queueDesc));
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ID3D12CommandQueue* commandQueue;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue))),
            "Failed create command queue.");
        commandQueue_ = makeComUnique(commandQueue);

        // Create the swap chain
        RECT clientRect;
        GetClientRect(window, &clientRect);
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
        swapChainDesc.OutputWindow = window;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;

        IDXGIFactory4* factory;
        enforce<Direct3DException>(SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))),
            "Failed to create DXGI factory.");
        const auto factoryHolder = makeComUnique(factory);

        IDXGISwapChain* swapChain0;
        enforce<Direct3DException>(SUCCEEDED(factory->CreateSwapChain(commandQueue_.get(), &swapChainDesc, &swapChain0)),
            "Failed to create swap chain.");
        const auto swapChain0Holder = makeComUnique(swapChain0);

        IDXGISwapChain3* swapChain3;
        enforce<Direct3DException>(SUCCEEDED(swapChain0->QueryInterface(IID_PPV_ARGS(&swapChain3))),
            "Faild to gat IDXGISwapChain3.");
        swapChain_ = makeComUnique(swapChain3);

        frameIndex_ = swapChain_->GetCurrentBackBufferIndex();

        // Create the descripter heap for render target views
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
        ZeroMemory(&rtvHeapDesc, sizeof(rtvHeapDesc));
        rtvHeapDesc.NumDescriptors = NUM_BACK_BUFFERS;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        ID3D12DescriptorHeap* rtvHeap;
        enforce<Direct3DException>(SUCCEEDED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap))),
            "Failed to create descripter heap of render target view.");
        rtvHeap_ = makeComUnique(rtvHeap);
        rtvSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create render target views
        auto rtv = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            ID3D12Resource* renderTarget;
            enforce<Direct3DException>(SUCCEEDED(swapChain_->GetBuffer(i, IID_PPV_ARGS(&renderTarget))),
                "Failed to get back buffer.");
            device->CreateRenderTargetView(renderTarget, nullptr, rtv);

            renderTargets_[i] = std::make_shared<RenderTarget>(renderTarget, rtv);
            rtv.ptr += rtvSize_;
        }

        // Create command allocator
        ID3D12CommandAllocator* commandAllocator;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator))),
            "Failed to create command allocator.");
        commandAllocator_ = makeComUnique(commandAllocator);

        // Create the fence
        ID3D12Fence* fence;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))),
            "Failed to create fence.");
        fence_ = makeComUnique(fence);
        fenceEvent_ = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        fenceValue_ = 1;
    }

    std::shared_ptr<RenderTarget> RenderSystem::getCurrentBackBuffer()
    {
        return renderTargets_[frameIndex_];
    }

    std::shared_ptr<VertexBuffer> RenderSystem::createVertexBuffer(const void* data, size_t size, size_t stride)
    {
        assert(stride <= size && "You need stride <= size.");

        /// TODO: Now, Only use upload heap. We can use default heap to store data for optimization.
        // Use upload heap
        D3D12_HEAP_PROPERTIES uploadHeapProps;
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        uploadHeapProps.CreationNodeMask = 1;
        uploadHeapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = size;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // Create buffer
        ID3D12Resource* buffer;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create vertex buffer.");

        // Upload data to buffer
        void* mappedData;
        enforce<Direct3DException>(SUCCEEDED(buffer->Map(0, nullptr, &mappedData)),
            "Failed to map vertices memory.");

        std::memcpy(mappedData, data, static_cast<size_t>(desc.Width));
        buffer->Unmap(0, nullptr);

        return std::make_shared<VertexBuffer>(buffer, size, stride);
    }

    std::shared_ptr<IndexBuffer> RenderSystem::createIndexBuffer(const unsigned short* data, size_t size)
    {
        /// TODO: Now, Only use upload heap. We can use default heap to store data for optimization.
        // Use upload heap
        D3D12_HEAP_PROPERTIES uploadHeapProps;
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        uploadHeapProps.CreationNodeMask = 1;
        uploadHeapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = size;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // Create buffer
        ID3D12Resource* buffer;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create vertex buffer.");

        // Upload data to buffer
        void* mappedData;
        enforce<Direct3DException>(SUCCEEDED(buffer->Map(0, nullptr, &mappedData)),
            "Failed to map vertices memory.");

        std::memcpy(mappedData, data, static_cast<size_t>(desc.Width));
        buffer->Unmap(0, nullptr);

        return std::make_shared<IndexBuffer>(buffer, size);
    }

    std::shared_ptr<ConstantBuffer> RenderSystem::createConstantBuffer(size_t dataSize)
    {
        /// TODO: Now, Only use upload heap. We can use default heap to store data for optimization.
        // Use upload heap
        D3D12_HEAP_PROPERTIES uploadHeapProps;
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        uploadHeapProps.CreationNodeMask = 1;
        uploadHeapProps.VisibleNodeMask = 1;

        const size_t bufferSize = dataSize + 256 - (dataSize % 256);

        D3D12_RESOURCE_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = bufferSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // Create buffer
        ID3D12Resource* buffer;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create constant buffer.");

        return std::make_shared<ConstantBuffer>(buffer, bufferSize, dataSize);
    }

    std::shared_ptr<ResourceHeap> RenderSystem::createResourceHeap(size_t numResources)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.NumDescriptors = numResources;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        ID3D12DescriptorHeap* heap;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))),
            "Failed to create descripter heap.");

        return std::make_shared<ResourceHeap>(heap, desc.Type);
    }

    std::shared_ptr<RootSignature> RenderSystem::createRootSignature(RootSignatureDescription& desc)
    {
        const auto d3dDesc = desc.getD3DDescription();

        ID3DBlob* signature;
        ID3DBlob* err;
        const auto hr = D3D12SerializeRootSignature(&d3dDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &err);
        if (FAILED(hr))
        {
            std::string msg = "Failed to serialize root signature.";
            if (err)
            {
                msg += "\n";
                msg += static_cast<char*>(err->GetBufferPointer());
                err->Release();
            }
            throw Direct3DException(msg);
        }
        const auto signatureHolder = makeComUnique(signature);

        ID3D12RootSignature* rootSignature;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature))),
            "Failed to create root sigature.");

        return std::make_shared<RootSignature>(rootSignature);
    }

    std::shared_ptr<PipelineState> RenderSystem::createPipelineState(const PipelineStateDescription& stateDesc)
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

        D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dStateDesc;
        ZeroMemory(&d3dStateDesc, sizeof(d3dStateDesc));
        d3dStateDesc.InputLayout = stateDesc.vertexShader->getD3DInputLayout();
        d3dStateDesc.pRootSignature = stateDesc.rootSignature->getD3DRootSignature();
        d3dStateDesc.VS = {stateDesc.vertexShader->getByteCode(), stateDesc.vertexShader->getByteCodeSize()};
        d3dStateDesc.PS = {stateDesc.pixelShader->getByteCode(), stateDesc.pixelShader->getByteCodeSize()};
        d3dStateDesc.RasterizerState = rasterizerState;
        d3dStateDesc.BlendState = blendState;
        d3dStateDesc.DepthStencilState.DepthEnable = FALSE;
        d3dStateDesc.DepthStencilState.StencilEnable = FALSE;
        d3dStateDesc.SampleMask = UINT_MAX;
        d3dStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        d3dStateDesc.NumRenderTargets = 1;
        d3dStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        d3dStateDesc.SampleDesc.Count = 1;

        ID3D12PipelineState* pipelineState;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateGraphicsPipelineState(&d3dStateDesc, IID_PPV_ARGS(&pipelineState))),
            "Failed to create pipeline state.");

        return std::make_shared<PipelineState>(pipelineState);
    }

    std::shared_ptr<CommandList> RenderSystem::createCommandList()
    {
        ID3D12GraphicsCommandList* list;
        enforce<Direct3DException>(SUCCEEDED(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.get(), nullptr, IID_PPV_ARGS(&list))),
            "Failed to create command list.");
        list->Close();
        return std::make_shared<CommandList>(list);
    }

    void RenderSystem::storeResource(const std::shared_ptr<ConstantBuffer>& resource, const std::shared_ptr<ResourceHeap>& heap, size_t i)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc;
        viewDesc.BufferLocation = resource->getGPUAddress();
        viewDesc.SizeInBytes = resource->getBufferSize();

        const auto d3dHeap = heap->getD3DHeap();
        const auto heapType = heap->getType();
        const auto offset = device_->GetDescriptorHandleIncrementSize(heapType) * i;

        auto location = d3dHeap->GetCPUDescriptorHandleForHeapStart();
        location.ptr += offset;

        device_->CreateConstantBufferView(&viewDesc, location);
    }

    void RenderSystem::startCommandRecording()
    {
        enforce<Direct3DException>(SUCCEEDED(commandAllocator_->Reset()),
            "Failed to reset command allocator.");
    }

    void RenderSystem::resetCommandList(const std::shared_ptr<CommandList>& list, const std::shared_ptr<PipelineState>& pipelineState)
    {
        enforce<Direct3DException>(SUCCEEDED(list->getD3DCommandList()->Reset(commandAllocator_.get(), pipelineState->getD3DPipelineState())),
            "Faild to reset command list.");
    }

    void RenderSystem::executeCommandList(const std::shared_ptr<CommandList>& list)
    {
        ID3D12CommandList* d3dLists[] = {list->getD3DCommandList()};
        commandQueue_->ExecuteCommandLists(1, d3dLists);
    }

    void RenderSystem::presentBackBuffer()
    {
        // Wait for GPU draw finished
        enforce<Direct3DException>(SUCCEEDED(commandQueue_->Signal(fence_.get(), fenceValue_)),
            "Failed to signal draw end.");
        if (fence_->GetCompletedValue() < fenceValue_)
        {
            enforce<Direct3DException>(SUCCEEDED(fence_->SetEventOnCompletion(fenceValue_, fenceEvent_)),
                "Failed to set signal event.");
            WaitForSingleObject(fenceEvent_, INFINITE);
        }
        ++fenceValue_;

        // Flip screen
        enforce<Direct3DException>(SUCCEEDED(swapChain_->Present(1, 0)),
            "Failed to present back buffer.");

        // Update frame index
        frameIndex_ = swapChain_->GetCurrentBackBufferIndex();
    }
}