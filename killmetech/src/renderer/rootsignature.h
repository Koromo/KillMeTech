#ifndef _KILLME_ROOTSIGNATURE_H_
#define _KILLNE_ROOTSIGNATURE_H_

#include "../windows/winsupport.h"
#include <d3d12.h>
#include <vector>

namespace killme
{
    enum class ShaderType;

    enum class GpuResourceRangeType
    {
        cbv,
        srv,
        sampler
    };

    /** The range of gpu resources */
    class GpuResourceRange
    {
    private:
        D3D12_DESCRIPTOR_RANGE& range_;

    public:
        /** Constructs with a refference of the Direct3D range */
        explicit GpuResourceRange(D3D12_DESCRIPTOR_RANGE& range);

        /** Set a range value */
        void as(GpuResourceRangeType type, size_t baseRegister, size_t numResources, size_t offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
    };

    /** The root parameter */
    class RootParameter
    {
    private:
        D3D12_ROOT_PARAMETER& param_;
        std::vector<D3D12_DESCRIPTOR_RANGE> d3dRanges_;
        std::vector<GpuResourceRange> ranges_;

    public:
        /** Constructs with a refference of the Direct3D root parameter */
        explicit RootParameter(D3D12_ROOT_PARAMETER& param);

        /** Accesses the i'th range */
        GpuResourceRange& operator[](size_t i);

        /** Initializes */
        void asTable(size_t numRanges, ShaderType visibility);
    };

    /** The root signature description */
    class RootSignatureDescription
    {
    private:
        D3D12_ROOT_SIGNATURE_DESC desc_;
        std::vector<D3D12_ROOT_PARAMETER> d3dParams_;
        std::vector<RootParameter> params_;

    public:
        /** Constructs */
        explicit RootSignatureDescription(size_t numParams);

        /** Accesses the i'th root parameter */
        RootParameter& operator[](size_t i);

        /** Returns the Direct3D root signature description */
        D3D12_ROOT_SIGNATURE_DESC getD3DDescription();
    };

    /** The root signature */
    class RootSignature
    {
    private:
        ComUniquePtr<ID3D12RootSignature> rootSignature_;

    public:
        /** Constructs with a Direct3D root signature */
        explicit RootSignature(ID3D12RootSignature* rootSignature);

        /** Returns the Direct3D root signature */
        ID3D12RootSignature* getD3DRootSignature();
    };
}

#endif