#include "shader.h"
#include <cstring>
#include <cassert>

namespace killme
{
    D3D_SHADER_INPUT_TYPE detail::toD3DSIType(BoundResourceType type)
    {
        switch (type)
        {
        case BoundResourceType::cbuffer: return D3D_SIT_CBUFFER;
        case BoundResourceType::texture: return D3D_SIT_TEXTURE;
        case BoundResourceType::sampler: return D3D_SIT_SAMPLER;
        default:
            assert(false && "Item not found.");
            return D3D_SIT_CBUFFER; // For warnings
        }
    }

    namespace
    {
        BoundResourceType toBoundResourceType(D3D_SHADER_INPUT_TYPE type)
        {
            switch (type)
            {
            case D3D_SIT_CBUFFER: return BoundResourceType::cbuffer;
            case D3D_SIT_TEXTURE: return BoundResourceType::texture;
            case D3D_SIT_SAMPLER: return BoundResourceType::sampler;
            default:
                assert(false && "Item not found.");
                return BoundResourceType::cbuffer; // For warnings
            }
        }
    }

    BoundResourceDescription::BoundResourceDescription(const D3D12_SHADER_INPUT_BIND_DESC& desc)
        : desc_(desc)
    {
    }

    BoundResourceType BoundResourceDescription::getType() const
    {
        return toBoundResourceType(desc_.Type);
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

    const void* BasicShader::getByteCode() const
    {
        return byteCode_->GetBufferPointer();
    }

    size_t BasicShader::getByteCodeSize() const
    {
        return byteCode_->GetBufferSize();
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
}