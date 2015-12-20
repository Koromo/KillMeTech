#ifndef _KILLME_ROOTSIGNATURE_H_
#define _KILLNE_ROOTSIGNATURE_H_

#include "../windows/winsupport.h"
#include <d3d12.h>

namespace killme
{
    /** Root signature */
    class RootSignature
    {
    private:
        ComUniquePtr<ID3D12RootSignature> rootSignature_;

    public:
        /** Construct with a Direct3D root signature */
        explicit RootSignature(ID3D12RootSignature* rootSignature);

        /** Returns Direct3D root signature */
        ID3D12RootSignature* getD3DRootSignature();
    };
}

#endif