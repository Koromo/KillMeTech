#include "rendersystem.h"
#include "commandallocator.h"
#include "commandlist.h"
#include "commandqueue.h"
#include "gpuresourceheap.h"
#include "vertexdata.h"
#include "constantbuffer.h"
//#include "pixels.h"
//#include "image.h"
#include "texture.h"
#include "rootsignature.h"
#include "pipelinestate.h"
#include "shaders.h"
#include "inputlayout.h"
#include "d3dsupport.h"
#include "../resources/resource.h"
#include "../core/math/math.h"
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
        , swapChain_()
        , frameIndex_()
        , backBufferHeap_()
        , backBuffers_()
        , backBufferViews_()
        , depthStencilHeap_()
        , depthStencil_()
        , depthStencilView_()
        , readyAllocators_()
        , queuedAllocators_()
        , readyLists_()
        , queuedLists_()
        , commandQueue_()
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
        auto safeQueue = makeComUnique(commandQueue);

        ID3D12Fence* fence;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))),
            "Failed to create the fence.");

        commandQueue_ = std::make_shared<CommandQueue>(safeQueue.release(), fence);

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
            SUCCEEDED(factory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChain0)),
            "Failed to create the swap chain.");
        KILLME_SCOPE_EXIT{ swapChain0->Release(); };

        IDXGISwapChain3* swapChain3;
        enforce<Direct3DException>(
            SUCCEEDED(swapChain0->QueryInterface(IID_PPV_ARGS(&swapChain3))),
            "Faild to gat IDXGISwapChain3.");
        swapChain_ = makeComUnique(swapChain3);

        frameIndex_ = swapChain_->GetCurrentBackBufferIndex();

        // Create render targets from back buffers
        backBufferHeap_ = createGpuResourceHeap(NUM_BACK_BUFFERS, GpuResourceHeapType::rtv, GpuResourceHeapFlag::none);
        for (UINT i = 0; i < NUM_BACK_BUFFERS; ++i)
        {
            ID3D12Resource* backBuffer;
            enforce<Direct3DException>(
                SUCCEEDED(swapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffer))),
                "Failed to get the back buffer.");
            backBuffers_[i] = std::make_shared<RenderTarget>(backBuffer);
            backBufferViews_[i] = backBufferHeap_->createView(i, backBuffers_[i]);
        }

        // Create the depth stencil
        depthStencilHeap_ = createGpuResourceHeap(1, GpuResourceHeapType::dsv, GpuResourceHeapFlag::none);

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
        depthStencilView_ = depthStencilHeap_->createView(0, depthStencil_);
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

    std::shared_ptr<VertexBuffer> RenderSystem::createVertexBuffer(size_t size, size_t stride)
    {
        assert(stride <= size && "You need satisfy stride <= size.");

        const auto defaultHeapProps = getD3DDefaultHeapProps();
        const auto desc = describeD3DBuffer(size);

        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the vertex buffer.");

        return std::make_shared<VertexBuffer>(buffer, stride);
    }

    std::shared_ptr<IndexBuffer> RenderSystem::createIndexBuffer(size_t size)
    {
        const auto defaultHeapProps = getD3DDefaultHeapProps();
        const auto desc = describeD3DBuffer(size);

        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the index buffer.");

        return std::make_shared<IndexBuffer>(buffer);
    }

    std::shared_ptr<ConstantBuffer> RenderSystem::createConstantBuffer(size_t size)
    {
        /// TODO: Now, Only use the upload heap. We can use the default heap to store the data.
        // Use the upload heap
        const auto bufferSize = ceiling(size, 256u);
        const auto uploadHeapProps = getD3DUploadHeapProps();
        const auto desc = describeD3DBuffer(bufferSize);

        // Create the buffer
        ID3D12Resource* buffer;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))),
            "Failed to create the constant buffer.");

        return std::make_shared<ConstantBuffer>(buffer);
    }

    std::shared_ptr<Texture> RenderSystem::createTexture(size_t width, size_t height, PixelFormat format)
    {
        const auto defaultHeapProps = getD3DDefaultHeapProps();
        const auto desc = describeD3DTex2D(width, height, toD3DPixelFormat(format), D3D12_RESOURCE_FLAG_NONE);

        ID3D12Resource* tex;
        enforce<Direct3DException>(
            SUCCEEDED(device_->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&tex))),
            "Failed to create the texture.");

        return std::make_shared<Texture>(tex, format);
    }

    namespace
    {
        D3D12_DESCRIPTOR_HEAP_TYPE toD3DHeapType(GpuResourceHeapType type)
        {
            switch (type)
            {
            case GpuResourceHeapType::rtv: return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            case GpuResourceHeapType::dsv: return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            case GpuResourceHeapType::cbv_srv: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case GpuResourceHeapType::sampler: return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
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

    namespace
    {
        D3D12_BLEND toD3DBlend(Blend b)
        {
            switch (b)
            {
            case Blend::one: return D3D12_BLEND_ONE;
            case Blend::zero: return D3D12_BLEND_ZERO;
            default:
                assert(false && "Item not found.");
                return D3D12_BLEND_ONE;
            }
        }

        D3D12_BLEND_OP toD3DBlendOp(BlendOp op)
        {
            switch (op)
            {
            case BlendOp::add: return D3D12_BLEND_OP_ADD;
            case BlendOp::subtract: return D3D12_BLEND_OP_SUBTRACT;
            case BlendOp::min: return D3D12_BLEND_OP_MIN;
            case BlendOp::max: return D3D12_BLEND_OP_MAX;
            default:
                assert(false && "Item not found.");
                return D3D12_BLEND_OP_ADD;
            }
        }

        D3D12_RENDER_TARGET_BLEND_DESC toD3DBlendState(const BlendState& blend)
        {
            return{
                blend.enable, FALSE,
                toD3DBlend(blend.src), toD3DBlend(blend.dest), toD3DBlendOp(blend.op),
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP,
                D3D12_COLOR_WRITE_ENABLE_ALL
            };
        }
    }

    std::shared_ptr<PipelineState> RenderSystem::createPipelineState(const PipelineStateDescription& pipelineDesc)
    {
        assert((pipelineDesc.vertexShader.bound() && pipelineDesc.pixelShader.bound()) &&
            "Vertex shader and pixel shader absolutely necessity for create pipeline state.");

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
        blendState.RenderTarget[0] = toD3DBlendState(pipelineDesc.blend);

        /*
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRTBlendDesc = {
            FALSE, FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };
        */
        for (UINT i = 1; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            blendState.RenderTarget[i] = toD3DBlendState(BlendState::DEFAULT);
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
        d3dDesc.VS = pipelineDesc.vertexShader.access()->getD3DByteCode();
        d3dDesc.PS = pipelineDesc.pixelShader.access()->getD3DByteCode();
        d3dDesc.GS = pipelineDesc.geometryShader.bound() ? pipelineDesc.geometryShader.access()->getD3DByteCode() : d3dDesc.GS;
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

    std::shared_ptr<CommandAllocator> RenderSystem::obtainCommandAllocator()
    {
        commandQueue_->updateExecutionState();

        auto it = std::cbegin(queuedAllocators_);
        while (it != std::cend(queuedAllocators_))
        {
            if ((*it)->isLocked())
            {
                ++it;
            }
            else
            {
                (*it)->reset();
                readyAllocators_.emplace(*it);
                it = queuedAllocators_.erase(it);
            }
        }

        if (readyAllocators_.empty())
        {
            ID3D12CommandAllocator* allocator;
            enforce<Direct3DException>(
                SUCCEEDED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))),
                "Failed to create the CommandAllocator.");
            const auto front = std::make_shared<CommandAllocator>(allocator);
            front->reset();
            readyAllocators_.emplace(front);
        }

        const auto front = readyAllocators_.front();
        readyAllocators_.pop();

        return front;
    }

    void RenderSystem::reuseCommandAllocator(const std::shared_ptr<CommandAllocator>& allocator)
    {
        readyAllocators_.emplace(allocator);
    }

    void RenderSystem::reuseCommandAllocatorAfterExecution(const std::shared_ptr<CommandAllocator>& allocator)
    {
        queuedAllocators_.emplace_back(allocator);
    }

    std::shared_ptr<CommandList> RenderSystem::obtainCommandList(const std::shared_ptr<CommandAllocator>& allocator, const std::shared_ptr<PipelineState>& pipeline)
    {
        commandQueue_->updateExecutionState();

        auto it = std::cbegin(queuedLists_);
        while (it != std::cend(queuedLists_))
        {
            if ((*it)->isLocked())
            {
                ++it;
            }
            else
            {
                readyLists_.emplace(*it);
                it = queuedLists_.erase(it);
            }
        }

        if (readyLists_.empty())
        {
            /// TODO: Initial pipeline
            const auto d3dAllocator = allocator->getD3DAllocator();
            ID3D12GraphicsCommandList* list;
            enforce<Direct3DException>(
                SUCCEEDED(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3dAllocator, nullptr, IID_PPV_ARGS(&list))),
                "Failed to create the command list.");

            const auto front = std::make_shared<CommandList>(list, allocator);
            front->close();
            readyLists_.emplace(front);
        }

        const auto front = readyLists_.front();
        readyLists_.pop();

        front->reset(allocator, pipeline);
        return front;
    }

    void RenderSystem::reuseCommandList(const std::shared_ptr<CommandList>& list)
    {
        readyLists_.emplace(list);
    }

    void RenderSystem::reuseCommandListAfterExecution(const std::shared_ptr<CommandList>& list)
    {
        queuedLists_.emplace_back(list);
    }

    std::shared_ptr<CommandQueue> RenderSystem::getCommandQueue()
    {
        return commandQueue_;
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