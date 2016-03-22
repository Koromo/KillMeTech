#include "pipelinestate.h"
#include "vertexdata.h"
#include "d3dsupport.h"
#include "../core/math/math.h"
#include "../core/platform.h"
#include <algorithm>
#include <cassert>

namespace killme
{
     GpuResourceHeapRequire::GpuResourceHeapRequire(GpuResourceHeapType type, size_t numResources)
         : rootIndices_()
         , type_(type)
         , require_(numResources)
     {
     }

     void GpuResourceHeapRequire::addRootIndex(size_t i)
     {
         rootIndices_.emplace_back(i);
     }

    GpuResourceHeapType GpuResourceHeapRequire::getType() const
    {
        return type_;
    }

    size_t GpuResourceHeapRequire::getNumResources() const
    {
        return require_.size();
    }

    const ShaderBoundRequire& GpuResourceHeapRequire::getResource(size_t i) const
    {
        assert(i < require_.size() && "Index out of range.");
        return require_[i];
    }

    ShaderBoundRequire& GpuResourceHeapRequire::getResource(size_t i)
    {
        return const_cast<ShaderBoundRequire&>(static_cast<const GpuResourceHeapRequire&>(*this).getResource(i));
    }

    GpuResourceTable::GpuResourceTable(const detail::BoundShaders& boundShaders)
        : heapTable_()
        , d3dHeapUniqueArray_()
        , d3dHeapTable_()
        , require_()
        , descriptorRanges_()
        , rootParams_()
        , rootSignature_()
    {
        // Create priority of each shaders
        std::vector<std::shared_ptr<BasicShader>> shaderPriority;
        if (boundShaders.vs.bound())
        {
            shaderPriority.emplace_back(boundShaders.vs.access());
        }
        if (boundShaders.ps.bound())
        {
            shaderPriority.emplace_back(boundShaders.ps.access());
        }
        if (boundShaders.gs.bound())
        {
            shaderPriority.emplace_back(boundShaders.gs.access());
        }

        // Reserve memory for D3D12_DESCRIPTOR_RANGEs
        size_t sumResources = 0;
        for (const auto& shader : shaderPriority)
        {
            sumResources += shader->getNumBoundResources();
        }
        descriptorRanges_.reserve(sumResources);

        // Create priority of required heap types
        const auto heapTypePriority = { GpuResourceHeapType::cbufferAndTexture, GpuResourceHeapType::sampler };
        size_t rangeCount = 0;

        for (const auto heapType : heapTypePriority)
        {
            // Initialize for heap of GpuResourceHeapType::cbufferAndTexture
            if (heapType == GpuResourceHeapType::cbufferAndTexture)
            {
                size_t numResources = 0;
                for (const auto& shader : shaderPriority)
                {
                    const auto& cbuffers = shader->describeBoundResources(BoundResourceType::cbuffer);
                    const auto& textures = shader->describeBoundResources(BoundResourceType::texture);
                    const auto numCBuffers = std::cend(cbuffers) - std::cbegin(cbuffers);
                    const auto numTextures = std::cend(textures) - std::cbegin(textures);
                    numResources += numCBuffers + numTextures;
                }

                if (numResources == 0)
                {
                    continue;
                }

                GpuResourceHeapRequire requiredHeap(GpuResourceHeapType::cbufferAndTexture, numResources);
                size_t resourceIndex = 0;

                for (const auto& shader : shaderPriority)
                {
                    const auto& cbuffers = shader->describeConstnatBuffers();
                    const auto& textures = shader->describeBoundResources(BoundResourceType::texture);

                    for (const auto& cbuffer : cbuffers)
                    {
                        requiredHeap.getResource(resourceIndex).boundShader = shader;
                        requiredHeap.getResource(resourceIndex).type = BoundResourceType::cbuffer;
                        requiredHeap.getResource(resourceIndex).cbuffer = cbuffer;

                        D3D12_DESCRIPTOR_RANGE range;
                        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                        range.BaseShaderRegister = cbuffer.getRegisterSlot();
                        range.NumDescriptors = 1;
                        range.OffsetInDescriptorsFromTableStart = resourceIndex;
                        range.RegisterSpace = 0;
                        descriptorRanges_.emplace_back(std::move(range));

                        ++resourceIndex;
                    }
                    for (const auto& texture : textures)
                    {
                        requiredHeap.getResource(resourceIndex).boundShader = shader;
                        requiredHeap.getResource(resourceIndex).type = BoundResourceType::texture;
                        requiredHeap.getResource(resourceIndex).texture = texture;

                        D3D12_DESCRIPTOR_RANGE range;
                        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                        range.BaseShaderRegister = texture.getRegisterSlot();
                        range.NumDescriptors = 1;
                        range.OffsetInDescriptorsFromTableStart = resourceIndex;
                        range.RegisterSpace = 0;
                        descriptorRanges_.emplace_back(std::move(range));

                        ++resourceIndex;
                    }

                    const auto rangeCountLocal = descriptorRanges_.size() - rangeCount;
                    if (rangeCountLocal > 0)
                    {
                        D3D12_ROOT_PARAMETER rootParam;
                        rootParam.ShaderVisibility = D3DMappings::toD3DShaderVisibility(shader->getType());
                        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                        rootParam.DescriptorTable.NumDescriptorRanges = rangeCountLocal;
                        rootParam.DescriptorTable.pDescriptorRanges = descriptorRanges_.data() + rangeCount;
                        rootParams_.emplace_back(std::move(rootParam));

                        requiredHeap.addRootIndex(rootParams_.size() - 1);
                        rangeCount = descriptorRanges_.size();
                    }
                }

                require_.emplace_back(std::move(requiredHeap));
            }
            else if (heapType == GpuResourceHeapType::sampler)
            {
                size_t numResources = 0;
                for (const auto& shader : shaderPriority)
                {
                    const auto& samplers = shader->describeBoundResources(BoundResourceType::sampler);
                    const auto numSamplers = std::cend(samplers) - std::cbegin(samplers);
                    numResources += numSamplers;
                }

                if (numResources == 0)
                {
                    continue;
                }

                GpuResourceHeapRequire requiredHeap(GpuResourceHeapType::sampler, numResources);
                size_t resourceIndex = 0;

                for (const auto& shader : shaderPriority)
                {
                    const auto& samplers = shader->describeBoundResources(BoundResourceType::sampler);
                    for (const auto& sampler : samplers)
                    {
                        requiredHeap.getResource(resourceIndex).boundShader = shader;
                        requiredHeap.getResource(resourceIndex).type = BoundResourceType::sampler;
                        requiredHeap.getResource(resourceIndex).sampler = sampler;

                        D3D12_DESCRIPTOR_RANGE range;
                        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                        range.BaseShaderRegister = sampler.getRegisterSlot();
                        range.NumDescriptors = 1;
                        range.OffsetInDescriptorsFromTableStart = resourceIndex;
                        range.RegisterSpace = 0;
                        descriptorRanges_.emplace_back(std::move(range));

                        ++resourceIndex;
                    }

                    const auto rangeCountLocal = descriptorRanges_.size() - rangeCount;
                    if (rangeCountLocal > 0)
                    {
                        D3D12_ROOT_PARAMETER rootParam;
                        rootParam.ShaderVisibility = D3DMappings::toD3DShaderVisibility(shader->getType());
                        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                        rootParam.DescriptorTable.NumDescriptorRanges = rangeCountLocal;
                        rootParam.DescriptorTable.pDescriptorRanges = descriptorRanges_.data() + rangeCount;
                        rootParams_.emplace_back(std::move(rootParam));

                        requiredHeap.addRootIndex(rootParams_.size() - 1);
                        rangeCount = descriptorRanges_.size();
                    }
                }

                require_.emplace_back(std::move(requiredHeap));
            }
        }

        ZeroMemory(&rootSignature_, sizeof(rootSignature_));
        rootSignature_.pParameters = rootParams_.data();
        rootSignature_.NumParameters = rootParams_.size();
        rootSignature_.NumStaticSamplers = 0;
        rootSignature_.pStaticSamplers = nullptr;
        rootSignature_.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        heapTable_.resize(rootSignature_.NumParameters, nullptr);
        d3dHeapUniqueArray_.reserve(require_.size());
        d3dHeapTable_.resize(rootSignature_.NumParameters, nullptr);
    }

    size_t GpuResourceTable::getNumRequiredHeaps() const
    {
        return require_.size();
    }

    const GpuResourceHeapRequire& GpuResourceTable::getRequiredHeap(size_t i) const
    {
        assert(i < require_.size() && "Index out of range");
        return require_[i];
    }

    void GpuResourceTable::set(size_t i, const std::shared_ptr<GpuResourceHeap>& heap)
    {
        assert(i < rootSignature_.NumParameters && "Index out of range");

        if (heapTable_[i])
        {
            const auto begin = std::cbegin(d3dHeapUniqueArray_);
            const auto end = std::cend(d3dHeapUniqueArray_);
            const auto it = std::find(begin, end, d3dHeapTable_[i]);
            if (it != end)
            {
                d3dHeapUniqueArray_.erase(it);
            }
        }

        heapTable_[i] = heap;

        if (heap)
        {
            d3dHeapTable_[i] = heap->getD3DHeap();

            const auto begin = std::cbegin(d3dHeapUniqueArray_);
            const auto end = std::cend(d3dHeapUniqueArray_);
            const auto it = std::find(begin, end, d3dHeapTable_[i]);
            if (it == end)
            {
                d3dHeapUniqueArray_.emplace_back(d3dHeapTable_[i]);
            }
        }
        else
        {
            d3dHeapTable_[i] = nullptr;
        }
    }

    size_t GpuResourceTable::getRootSignatureHash() const
    {
        return crc32(&rootSignature_, sizeof(rootSignature_));
    }

    void GpuResourceTable::applyResourceTable(ID3D12GraphicsCommandList* commands)
    {
        commands->SetDescriptorHeaps(d3dHeapUniqueArray_.size(), d3dHeapUniqueArray_.data());
        for (size_t i = 0; i < rootSignature_.NumParameters; ++i)
        {
            if (d3dHeapTable_[i])
            {
                commands->SetGraphicsRootDescriptorTable(i, d3dHeapTable_[i]->GetGPUDescriptorHandleForHeapStart());
            }
        }
    }

    ID3D12RootSignature* GpuResourceTable::createD3DSignature(ID3D12Device* device) const
    {
        ID3DBlob* blob = NULL;
        ID3DBlob* err = NULL;
        const auto hr = D3D12SerializeRootSignature(&rootSignature_, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err);
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

        KILLME_SCOPE_EXIT{ blob->Release(); };

        ID3D12RootSignature* signature;
        enforce<Direct3DException>(
            SUCCEEDED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&signature))),
            "Failed to create the root sigature.");

        return signature;
    }

    void PipelineState::initialize()
    {
        boundShaders_.hashVS = -1;
        boundShaders_.hashPS = -1;
        boundShaders_.hashGS = -1;
        primitiveTopology_ = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

        ZeroMemory(&viewport_, sizeof(viewport_));
        ZeroMemory(&scissorRect_, sizeof(scissorRect_));
        ZeroMemory(&topLevelDesc_, sizeof(topLevelDesc_));

        topLevelDesc_.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        topLevelDesc_.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        topLevelDesc_.RasterizerState.FrontCounterClockwise = FALSE;
        topLevelDesc_.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        topLevelDesc_.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        topLevelDesc_.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        topLevelDesc_.RasterizerState.DepthClipEnable = TRUE;
        topLevelDesc_.RasterizerState.MultisampleEnable = FALSE;
        topLevelDesc_.RasterizerState.AntialiasedLineEnable = FALSE;
        topLevelDesc_.RasterizerState.ForcedSampleCount = 0;
        topLevelDesc_.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        topLevelDesc_.BlendState.AlphaToCoverageEnable = FALSE;
        topLevelDesc_.BlendState.IndependentBlendEnable = FALSE;
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            topLevelDesc_.BlendState.RenderTarget[i] = D3DMappings::toD3DBlendState(BlendState::DEFAULT);
            topLevelDesc_.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
        }

        topLevelDesc_.DepthStencilState.DepthEnable = TRUE;
        topLevelDesc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        topLevelDesc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        topLevelDesc_.DepthStencilState.StencilEnable = FALSE;
        topLevelDesc_.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        topLevelDesc_.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        topLevelDesc_.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        topLevelDesc_.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        topLevelDesc_.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        topLevelDesc_.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        topLevelDesc_.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        topLevelDesc_.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        topLevelDesc_.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        topLevelDesc_.DSVFormat = DXGI_FORMAT_UNKNOWN;

        topLevelDesc_.SampleMask = UINT_MAX;
        topLevelDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        topLevelDesc_.NumRenderTargets = 1;
        topLevelDesc_.SampleDesc.Count = 1;
    }

    void PipelineState::setVShader(const Resource<VertexShader>& vs)
    {
        boundShaders_.vs = vs;
        vertexBufferViews_.clear();

        if (!vs.bound())
        {
            boundShaders_.hashVS = -1;
        }
        else
        {
            const auto code = vs.access()->getD3DByteCode();
            boundShaders_.hashVS = crc32(code.pShaderBytecode, code.BytecodeLength);

            if (vertices_)
            {
                const auto& views = vertices_->getD3DVertexViews(vs.access());
                vertexBufferViews_.insert(std::cbegin(vertexBufferViews_), std::cbegin(views), std::cend(views));
            }
        }
        resourceTable_.reset();
    }

    void PipelineState::setPShader(const Resource<PixelShader>& ps)
    {
        boundShaders_.ps = ps;
        if (!ps.bound())
        {
            boundShaders_.hashPS = -1;
        }
        else
        {
            const auto code = ps.access()->getD3DByteCode();
            boundShaders_.hashPS = crc32(code.pShaderBytecode, code.BytecodeLength);
        }
        resourceTable_.reset();
    }

    void PipelineState::setGShader(const Resource<GeometryShader>& gs)
    {
        boundShaders_.gs = gs;
        if (!gs.bound())
        {
            boundShaders_.hashGS = -1;
        }
        else
        {
            const auto code = gs.access()->getD3DByteCode();
            boundShaders_.hashGS = crc32(code.pShaderBytecode, code.BytecodeLength);
        }
        resourceTable_.reset();
    }

    std::shared_ptr<GpuResourceTable> PipelineState::getGpuResourceTable()
    {
        createGpuResourceTableIf();
        return resourceTable_;
    }

    void PipelineState::setRenderTarget(Optional<RenderTarget::Location> rt, PixelFormat rtFormat,
        Optional<DepthStencil::Location> ds, PixelFormat dsFormat)
    {
        if (rt)
        {
            renderTarget_ = rt->ofD3D;
            topLevelDesc_.RTVFormats[0] = D3DMappings::toD3DDxgiFormat(rtFormat);
        }
        else
        {
            renderTarget_ = nullopt;
            topLevelDesc_.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
        }

        if (ds)
        {
            depthStencil_ = ds->ofD3D;
            topLevelDesc_.DSVFormat = D3DMappings::toD3DDxgiFormat(dsFormat);
        }
        else
        {
            depthStencil_ = nullopt;
            topLevelDesc_.DSVFormat = DXGI_FORMAT_UNKNOWN;
        }
    }

    void PipelineState::setVertexBuffers(const std::shared_ptr<VertexData>& vertices, bool setIndices)
    {
        vertices_ = vertices;
        vertexBufferViews_.clear();

        if (boundShaders_.vs.bound() && vertices_)
        {
            const auto& views = vertices_->getD3DVertexViews(boundShaders_.vs.access());
            vertexBufferViews_.insert(std::cbegin(vertexBufferViews_), std::cbegin(views), std::cend(views));
        }

        if (setIndices && vertices_)
        {
            const auto indices = vertices_->getIndexBuffer();
            if (indices)
            {
                indexBufferView_ = indices->getD3DView();
            }
            else
            {
                indexBufferView_ = nullopt;
            }
        }
    }

    void PipelineState::setPrimitiveTopology(PrimitiveTopology pt)
    {
        primitiveTopology_ = D3DMappings::toD3DPrimitiveTopology(pt);
        topLevelDesc_.PrimitiveTopologyType = D3DMappings::toD3DPrimitiveTopologyType(pt);
    }

    void PipelineState::setViewport(const Viewport& vp)
    {
        viewport_ = D3DMappings::toD3DViewport(vp);
    }

    void PipelineState::setScissorRect(const ScissorRect& sr)
    {
        scissorRect_ = D3DMappings::toD3DRect(sr);
    }

    void PipelineState::setBlendState(const BlendState& blend)
    {
        topLevelDesc_.BlendState.RenderTarget[0] = D3DMappings::toD3DBlendState(blend);
    }

    size_t PipelineState::getTopLevelHash() const
    {
        return crc32(&topLevelDesc_, sizeof(topLevelDesc_));
    }

    long PipelineState::getPSHash() const
    {
        return boundShaders_.hashPS;
    }

    long PipelineState::getVSHash() const
    {
        return boundShaders_.hashVS;
    }

    long PipelineState::getGSHash() const
    {
        return boundShaders_.hashGS;
    }

    size_t PipelineState::getRootSignatureHash() const
    {
        createGpuResourceTableIf();
        return resourceTable_->getRootSignatureHash();
    }

    void PipelineState::createGpuResourceTableIf() const
    {
        if (!resourceTable_)
        {
            resourceTable_ = std::make_shared<GpuResourceTable>(boundShaders_);
        }
    }

    void PipelineState::applyParameters(ID3D12GraphicsCommandList* commands)
    {
        resourceTable_->applyResourceTable(commands);

        const auto prtv = renderTarget_ ? renderTarget_.ptr() : nullptr;
        const auto pdsv = renderTarget_ ? depthStencil_.ptr() : nullptr;

        commands->OMSetRenderTargets(1, prtv, TRUE, pdsv);

        commands->IASetPrimitiveTopology(primitiveTopology_);
        commands->IASetVertexBuffers(0, vertexBufferViews_.size(), vertexBufferViews_.data());

        const auto pibv = indexBufferView_ ? indexBufferView_.ptr() : nullptr;
        commands->IASetIndexBuffer(pibv);

        commands->RSSetViewports(1, &viewport_);
        commands->RSSetScissorRects(1, &scissorRect_);
    }

    ID3D12PipelineState* PipelineState::createD3DPipeline(ID3D12Device* device, ID3D12RootSignature* rootSignature) const
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = topLevelDesc_;
        pipelineDesc.pRootSignature = rootSignature;
        pipelineDesc.InputLayout = boundShaders_.vs.bound() ? boundShaders_.vs.access()->getD3DInputLayout() : topLevelDesc_.InputLayout;
        pipelineDesc.VS = boundShaders_.vs.bound() ? boundShaders_.vs.access()->getD3DByteCode() : topLevelDesc_.VS;
        pipelineDesc.PS = boundShaders_.ps.bound() ? boundShaders_.ps.access()->getD3DByteCode() : topLevelDesc_.PS;
        pipelineDesc.GS = boundShaders_.gs.bound() ? boundShaders_.gs.access()->getD3DByteCode() : topLevelDesc_.GS;

        ID3D12PipelineState* pipeline;
        enforce<Direct3DException>(
            SUCCEEDED(device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&pipeline))),
            "Failed to create the pipeline state.");

        return pipeline;
    }

    ID3D12RootSignature* PipelineState::createD3DSignature(ID3D12Device* device) const
    {
        return resourceTable_->createD3DSignature(device);
    }

    ID3D12PipelineState* PipelineStateCache::getPipeline(ID3D12Device* device, const PipelineState& key)
    {
        const auto rootSigFound = pipelineMap_.map.find(key.getTopLevelHash());
        if (rootSigFound == std::cend(pipelineMap_.map))
        {
            return create(device, key);
        }

        const auto psFound = rootSigFound->second.map.find(key.getRootSignatureHash());
        if (psFound == std::cend(rootSigFound->second.map))
        {
            return create(device, key);
        }

        const auto vsFound = psFound->second.map.find(key.getPSHash());
        if (vsFound == std::cend(psFound->second.map))
        {
            return create(device, key);
        }

        const auto gsFound = vsFound->second.map.find(key.getVSHash());
        if (gsFound == std::cend(vsFound->second.map))
        {
            return create(device, key);
        }

        const auto found = gsFound->second.map.find(key.getGSHash());
        if (found == std::cend(gsFound->second.map))
        {
            return create(device, key);
        }
        
        return found->second.get();
    }

    ID3D12RootSignature* PipelineStateCache::getSignature(ID3D12Device* device, const PipelineState& key)
    {
        const auto it = signatureMap_.find(key.getRootSignatureHash());
        if (it == std::cend(signatureMap_))
        {
            const auto sig = key.createD3DSignature(device);
            signatureMap_.emplace(key.getRootSignatureHash(), makeComUnique(sig));
            return sig;
        }
        return it->second.get();
    }

    ID3D12PipelineState* PipelineStateCache::create(ID3D12Device* device, const PipelineState& key)
    {
        const auto signature = getSignature(device, key);
        const auto pipeline = key.createD3DPipeline(device, signature);

        pipelineMap_.map[key.getTopLevelHash()].map[key.getRootSignatureHash()].
            map[key.getPSHash()].map[key.getVSHash()].map[key.getGSHash()] = makeComUnique(pipeline);

        return pipeline;
    }
}