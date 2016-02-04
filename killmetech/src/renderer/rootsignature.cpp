#include "rootsignature.h"
#include "shader.h"
#include <cassert>

namespace killme
{
    RootParameter::RootParameter(D3D12_ROOT_PARAMETER& param)
        : param_(param)
        , range_()
    {
        // Setup range for constant buffers
        range_.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        range_.RegisterSpace = 0;
        range_.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        param_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param_.DescriptorTable.pDescriptorRanges = &range_;
        param_.DescriptorTable.NumDescriptorRanges = 1;
    }

    namespace
    {
        // Convert to D3D12_SHADER_VISIBILITY from ShaderType
        D3D12_SHADER_VISIBILITY toD3DShaderVisibility(ShaderType shader)
        {
            switch (shader)
            {
            case ShaderType::vertex: return D3D12_SHADER_VISIBILITY_VERTEX;
            case ShaderType::pixel: return D3D12_SHADER_VISIBILITY_PIXEL;
            default:
                assert(false && "An invalid ShaderType.");
                return D3D12_SHADER_VISIBILITY_ALL; // For warnings
            }
        }
    }

    void RootParameter::set(size_t baseRegister, size_t numResources, ShaderType visibility)
    {
        range_.NumDescriptors = numResources;
        range_.BaseShaderRegister = baseRegister;
        param_.ShaderVisibility = toD3DShaderVisibility(visibility);
    }

    RootSignatureDescription::RootSignatureDescription(size_t numParams)
        : desc_()
        , d3dParams_()
        , params_()
    {
        d3dParams_.resize(numParams);
        ZeroMemory(&desc_, sizeof(desc_));
        desc_.pParameters = d3dParams_.data();
        desc_.NumParameters = numParams;
        desc_.NumStaticSamplers = 0;
        desc_.pStaticSamplers = nullptr;
        desc_.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        params_.reserve(numParams);
        for (auto& d3dParam : d3dParams_)
        {
            ZeroMemory(&d3dParam, sizeof(d3dParam));
            params_.emplace_back(d3dParam);
        }
    }

    RootParameter& RootSignatureDescription::operator[](size_t i)
    {
        assert(i < desc_.NumParameters && "Index out of range.");
        return params_[i];
    }

    D3D12_ROOT_SIGNATURE_DESC RootSignatureDescription::getD3DDescription()
    {
        return desc_;
    }

    RootSignature::RootSignature(ID3D12RootSignature* rootSignature)
        : rootSignature_(makeComUnique(rootSignature))
    {
    }

    ID3D12RootSignature* RootSignature::getD3DRootSignature()
    {
        return rootSignature_.get();
    }
}