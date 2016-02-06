#include "shader.h"
#include "../core/exception.h"

namespace killme
{
    ConstantBufferDescription::ConstantBufferDescription(ID3D12ShaderReflectionConstantBuffer* reflection, size_t registerSlot)
        : name_()
        , registerSlot_(registerSlot)
        , size_()
        , variables_()
    {
        D3D12_SHADER_BUFFER_DESC desc;
        enforce<Direct3DException>(
            SUCCEEDED(reflection->GetDesc(&desc)),
            "Faild to get a description of constant buffer.");

        name_ = desc.Name;
        size_ = desc.Size;

        for (UINT i = 0; i < desc.Variables; ++i)
        {
            const auto d3dVarRef = reflection->GetVariableByIndex(i);

            D3D12_SHADER_VARIABLE_DESC d3dVarDesc;
            enforce<Direct3DException>(
                SUCCEEDED(d3dVarRef->GetDesc(&d3dVarDesc)),
                "Faild to get a description of constant variable.");

            VariableDescription varDesc;
            varDesc.size = d3dVarDesc.Size;
            varDesc.offset = d3dVarDesc.StartOffset;
            variables_.insert({d3dVarDesc.Name, varDesc});
        }
    }

    std::string ConstantBufferDescription::getName() const
    {
        return name_;
    }

    size_t ConstantBufferDescription::getSize() const
    {
        return size_;
    }

    size_t ConstantBufferDescription::getRegisterSlot() const
    {
        return registerSlot_;
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
        // Get shader reflection
        ID3D12ShaderReflection* reflection;
        enforce<Direct3DException>(
            SUCCEEDED(D3DReflect(byteCode_->GetBufferPointer(), byteCode_->GetBufferSize(), IID_PPV_ARGS(&reflection))),
            "Failed to get a reflection of shader.");
        reflection_ = makeComUnique(reflection);

        // Get shader description
        enforce<Direct3DException>(
            SUCCEEDED(reflection->GetDesc(&desc_)),
            "Failed to get a description of shader.");
    }

    const void* BasicShader::getByteCode() const
    {
        return byteCode_->GetBufferPointer();
    }

    size_t BasicShader::getByteCodeSize() const
    {
        return byteCode_->GetBufferSize();
    }

    std::vector<D3D12_SIGNATURE_PARAMETER_DESC> BasicShader::getD3DInputSignature()
    {
        std::vector<D3D12_SIGNATURE_PARAMETER_DESC> params(desc_.InputParameters);
        for (size_t i = 0; i < desc_.InputParameters; ++i)
        {
            enforce<Direct3DException>(
                SUCCEEDED(reflection_->GetInputParameterDesc(i, &(params[i]))),
                "Failed to get a description of input parameter of shader.");
        }
        return params;
    }

    std::vector<ConstantBufferDescription> BasicShader::describeConstnatBuffers()
    {
        std::vector<ConstantBufferDescription> cbuffers;
        cbuffers.reserve(desc_.ConstantBuffers);

        for (UINT i = 0; i < desc_.BoundResources; ++i)
        {
            D3D12_SHADER_INPUT_BIND_DESC resourceDesc;
            enforce<Direct3DException>(
                SUCCEEDED(reflection_->GetResourceBindingDesc(i, &resourceDesc)),
                "Failed to get resource description.");

            if (resourceDesc.Type == D3D_SIT_CBUFFER)
            {
                const auto cbuffer = enforce<Direct3DException>(
                    reflection_->GetConstantBufferByName(resourceDesc.Name),
                    "Failed to reflect constant buffer.");
                cbuffers.emplace_back(cbuffer, resourceDesc.BindPoint);
            }
        }

        return cbuffers;
    }
}