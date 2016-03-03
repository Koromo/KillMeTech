#include "rootsignature.h"
#include "shaders.h"
#include <cassert>

namespace killme
{
    GpuResourceRange::GpuResourceRange(D3D12_DESCRIPTOR_RANGE& range)
        : range_(range)
    {
        range_.RegisterSpace = 0;
    }

    namespace
    {
        D3D12_DESCRIPTOR_RANGE_TYPE toD3DRangeType(GpuResourceRangeType type)
        {
            switch (type)
            {
            case GpuResourceRangeType::cbv: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            case GpuResourceRangeType::srv: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            case GpuResourceRangeType::sampler: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            default:
                assert(false && "Item not found.");
                return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            }
        }
    }

    void GpuResourceRange::as(GpuResourceRangeType type, size_t baseRegister, size_t numResources, size_t offset)
    {
        range_.RangeType = toD3DRangeType(type);
        range_.NumDescriptors = numResources;
        range_.BaseShaderRegister = baseRegister;
        range_.OffsetInDescriptorsFromTableStart = offset;
    }

    RootParameter::RootParameter(D3D12_ROOT_PARAMETER& param)
        : param_(param)
        , d3dRanges_()
        , ranges_()
    {
    }

    GpuResourceRange& RootParameter::operator[](size_t i)
    {
        assert(i < ranges_.size() && "Index out of range.");
        return ranges_[i];
    }

    namespace
    {
        // Convert to the D3D12_SHADER_VISIBILITY from a ShaderType
        D3D12_SHADER_VISIBILITY toD3DShaderVisibility(ShaderType shader)
        {
            switch (shader)
            {
            case ShaderType::vertex: return D3D12_SHADER_VISIBILITY_VERTEX;
            case ShaderType::pixel: return D3D12_SHADER_VISIBILITY_PIXEL;
            case ShaderType::geometry: return D3D12_SHADER_VISIBILITY_GEOMETRY;
            default:
                assert(false && "An invalid ShaderType.");
                return D3D12_SHADER_VISIBILITY_ALL; // For warnings
            }
        }
    }

    void RootParameter::asTable(size_t numRanges, ShaderType visibility)
    {
        d3dRanges_.resize(numRanges);
        param_.ShaderVisibility = toD3DShaderVisibility(visibility);
        param_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param_.DescriptorTable.pDescriptorRanges = d3dRanges_.data();
        param_.DescriptorTable.NumDescriptorRanges = numRanges;

        ranges_.reserve(numRanges);
        for (auto& d3dRange : d3dRanges_)
        {
            ZeroMemory(&d3dRange, sizeof(d3dRange));
            ranges_.emplace_back(d3dRange);
        }
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