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
        enforce<Direct3DException>(
            SUCCEEDED(reflection->GetDesc(&bufferDesc)),
            "Faild to get the description of the constant buffer.");

        size_ = bufferDesc.Size;

        // Store descriptions of the variable
        for (UINT i = 0; i < bufferDesc.Variables; ++i)
        {
            const auto d3dVarRef = reflection->GetVariableByIndex(i);

            D3D12_SHADER_VARIABLE_DESC d3dVarDesc;
            enforce<Direct3DException>(
                SUCCEEDED(d3dVarRef->GetDesc(&d3dVarDesc)),
                "Faild to get a description of the constant variable.");

            VariableDescription varDesc;
            varDesc.size = d3dVarDesc.Size;
            varDesc.offset = d3dVarDesc.StartOffset;

            if (d3dVarDesc.DefaultValue != NULL)
            {
                const auto p = new unsigned char[d3dVarDesc.Size];
                std::memcpy(p, d3dVarDesc.DefaultValue, d3dVarDesc.Size);
                varDesc.defaultValue = std::shared_ptr<const unsigned char>(p, std::default_delete<const unsigned char[]>());
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

    BasicShader::BasicShader(ID3DBlob* byteCode)
        : byteCode_(makeComUnique(byteCode))
        , reflection_()
        , desc_()
    {
        // Get the reflection of the shader
        ID3D12ShaderReflection* reflection;
        enforce<Direct3DException>(
            SUCCEEDED(D3DReflect(byteCode_->GetBufferPointer(), byteCode_->GetBufferSize(), IID_PPV_ARGS(&reflection))),
            "Failed to get the reflection of the shader.");
        reflection_ = makeComUnique(reflection);

        // Get the description of the shader
        enforce<Direct3DException>(
            SUCCEEDED(reflection->GetDesc(&desc_)),
            "Failed to get the description of the shader.");
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
}