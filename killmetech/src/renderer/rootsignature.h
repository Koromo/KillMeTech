#ifndef _KILLME_ROOTSIGNATURE_H_
#define _KILLNE_ROOTSIGNATURE_H_

#include "../windows/winsupport.h"
#include <d3d12.h>
#include <vector>

namespace killme
{
    enum class ShaderType;

    /** Root parameter */
    class RootParameter
    {
    private:
        D3D12_ROOT_PARAMETER& param_;
        D3D12_DESCRIPTOR_RANGE range_;

    public:
        /** Constructs with a refference of Direct3D root parameter */
        explicit RootParameter(D3D12_ROOT_PARAMETER& param);

        /** Sets resource table */
        void set(size_t baseRegister, size_t numResources, ShaderType visibility);
    };

    /** Root signature description */
    class RootSignatureDescription
    {
    private:
        D3D12_ROOT_SIGNATURE_DESC desc_;
        std::vector<D3D12_ROOT_PARAMETER> d3dParams_;
        std::vector<RootParameter> params_;

    public:
        /** Constructs */
        explicit RootSignatureDescription(size_t numParams);

        /** Accesses i'th root parameter */
        RootParameter& operator[](size_t i);

        /** Returns a Direct3D root signature description */
        D3D12_ROOT_SIGNATURE_DESC getD3DDescription();
    };

    /** Root signature */
    class RootSignature
    {
    private:
        ComUniquePtr<ID3D12RootSignature> rootSignature_;

    public:
        /** Constructs  with a Direct3D root signature */
        explicit RootSignature(ID3D12RootSignature* rootSignature);

        /** Returns a Direct3D root signature */
        ID3D12RootSignature* getD3DRootSignature();
    };
}

#endif