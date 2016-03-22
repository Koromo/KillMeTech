#include "shaders.h"
#include <cstring>
#include <cassert>

namespace killme
{
    BoundResourceDescription::BoundResourceDescription(const D3D12_SHADER_INPUT_BIND_DESC& desc)
        : desc_(desc)
    {
    }

    BoundResourceType BoundResourceDescription::getType() const
    {
        return D3DMappings::toBoundResourceType(desc_.Type);
    }

    std::string BoundResourceDescription::getName() const
    {
        return desc_.Name;
    }

    size_t BoundResourceDescription::getRegisterSlot() const
    {
        return desc_.BindPoint;
    }

    ConstantBufferDescription::ConstantBufferDescription(ID3D12ShaderReflectionConstantBuffer* reflection, const D3D12_SHADER_INPUT_BIND_DESC& boundDesc)
        : BoundResourceDescription(boundDesc)
        , size_()
        , variables_()
    {
        // Get the description
        D3D12_SHADER_BUFFER_DESC bufferDesc;
        reflection->GetDesc(&bufferDesc);

        size_ = bufferDesc.Size;

        // Store descriptions of the variable
        for (UINT i = 0; i < bufferDesc.Variables; ++i)
        {
            const auto d3dVarRef = reflection->GetVariableByIndex(i);

            D3D12_SHADER_VARIABLE_DESC d3dVarDesc;
            d3dVarRef->GetDesc(&d3dVarDesc);

            VariableDescription varDesc;
            varDesc.size = d3dVarDesc.Size;
            varDesc.offset = d3dVarDesc.StartOffset;

            if (d3dVarDesc.DefaultValue != NULL)
            {
                const auto p = new unsigned char[d3dVarDesc.Size];
                std::memcpy(p, d3dVarDesc.DefaultValue, d3dVarDesc.Size);
                varDesc.init = std::shared_ptr<const unsigned char>(p, std::default_delete<const unsigned char[]>());
            }

            variables_.emplace(d3dVarDesc.Name, varDesc);
        }
    }

    size_t ConstantBufferDescription::getSize() const
    {
        return size_;
    }

    Optional<VariableDescription> ConstantBufferDescription::describeVariable(const std::string& name) const
    {
        const auto it = variables_.find(name);
        if (it == std::cend(variables_))
        {
            return nullopt;
        }
        return it->second;
    }

    BasicShader::BasicShader(ShaderType type, ID3DBlob* byteCode)
        : type_(type)
        , byteCode_(makeComUnique(byteCode))
        , reflection_()
        , desc_()
    {
        ID3D12ShaderReflection* reflection;
        D3DReflect(byteCode_->GetBufferPointer(), byteCode_->GetBufferSize(), IID_PPV_ARGS(&reflection));
        reflection_ = makeComUnique(reflection);

        reflection->GetDesc(&desc_);
    }

    ShaderType BasicShader::getType() const
    {
        return type_;
    }

    D3D12_SHADER_BYTECODE BasicShader::getD3DByteCode() const
    {
        return{ byteCode_->GetBufferPointer(), byteCode_->GetBufferSize() };
    }

    size_t BasicShader::getNumBoundResources() const
    {
        return desc_.BoundResources;
    }

    Optional<BoundResourceDescription> BasicShader::describeBoundResource(const std::string& name)
    {
        D3D12_SHADER_INPUT_BIND_DESC desc;
        const auto hr = reflection_->GetResourceBindingDescByName(name.c_str(), &desc);
        if (SUCCEEDED(hr))
        {
            return BoundResourceDescription(desc);
        }
        return nullopt;
    }

    Optional<ConstantBufferDescription> BasicShader::describeConstantBuffer(const std::string& name)
    {
        const auto cb = reflection_->GetConstantBufferByName(name.c_str());
        if (cb)
        {
            D3D12_SHADER_INPUT_BIND_DESC desc;
            reflection_->GetResourceBindingDescByName(name.c_str(), &desc);
            return ConstantBufferDescription(cb, desc);
        }
        return nullopt;
    }

    const std::string VertexShader::MODEL = "vs_5_0";
    const std::string PixelShader::MODEL = "ps_5_0";
    const std::string GeometryShader::MODEL = "gs_5_0";

    const std::string VertexShader::ENTRY = "vs_main";
    const std::string PixelShader::ENTRY = "ps_main";
    const std::string GeometryShader::ENTRY = "gs_main";

    namespace
    {
        // Return the corresponded vertex format
        DXGI_FORMAT getVertexFormat(const std::string& semanticName)
        {
            if (semanticName == "POSITION") { return DXGI_FORMAT_R32G32B32_FLOAT; }
            if (semanticName == "NORMAL") { return DXGI_FORMAT_R32G32B32_FLOAT; }
            if (semanticName == "TEXCOORD") { return DXGI_FORMAT_R32G32_FLOAT; }
            if (semanticName == "COLOR") { return DXGI_FORMAT_R32G32B32A32_FLOAT; }
            throw Direct3DException("Invalid vertex semantic name.");
        }
    }

    VertexShader::VertexShader(ID3DBlob* byteCode)
        : BasicShader(ShaderType::vertex, byteCode)
        , inputElems_()
        , inputLayout_()
    {
        // Collect input elements
        const auto signature = getD3DInputSignature();
        for (const auto& param : signature)
        {
            D3D12_INPUT_ELEMENT_DESC elem;
            elem.SemanticName = param.SemanticName;
            elem.SemanticIndex = param.SemanticIndex;
            elem.Format = getVertexFormat(param.SemanticName);
            elem.InputSlot = inputElems_.size();
            elem.AlignedByteOffset = 0;
            elem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elem.InstanceDataStepRate = 0;

            inputElems_.emplace_back(elem);
        }

        inputLayout_.NumElements = inputElems_.size();
        inputLayout_.pInputElementDescs = inputElems_.data();
    }

    D3D12_INPUT_LAYOUT_DESC VertexShader::getD3DInputLayout() const
    {
        return inputLayout_;
    }

    PixelShader::PixelShader(ID3DBlob* byteCode)
        : BasicShader(ShaderType::pixel, byteCode)
    {
    }

    GeometryShader::GeometryShader(ID3DBlob* byteCode)
        : BasicShader(ShaderType::geometry, byteCode)
    {
    }
}