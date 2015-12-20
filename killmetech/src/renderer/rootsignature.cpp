#include "rootsignature.h"

namespace killme
{
    RootSignature::RootSignature(ID3D12RootSignature* rootSignature)
        : rootSignature_(makeComUnique(rootSignature))
    {
    }

    ID3D12RootSignature* RootSignature::getD3DRootSignature()
    {
        return rootSignature_.get();
    }
}