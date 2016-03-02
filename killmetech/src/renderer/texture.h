#ifndef _KILLME_TEXTURE_H_
#define _KILLME_TEXTURE_H_

#include "rendersystem.h"
#include "image.h"
#include "../resources/resource.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <Windows.h>

namespace killme
{
    /** Texture resource */
    class Texture : public IsResource
    {
    private:
        ComUniquePtr<ID3D12Resource> tex_;
        D3D12_RESOURCE_DESC resourceDesc_;

    public:
        /** Resource view */
        struct View
        {
            D3D12_CPU_DESCRIPTOR_HANDLE d3dView;
        };

        /** Construct */
        explicit Texture(ID3D12Resource* tex)
            : tex_(makeComUnique(tex))
            , resourceDesc_(tex->GetDesc())
        {
        }

        /** Create the Direct3D view into a desctipror heap */
        View createD3DView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc;
            ZeroMemory(&desc, sizeof(desc));
            desc.Format = resourceDesc_.Format;
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipLevels = 1;
            desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            device->CreateShaderResourceView(tex_.get(), &desc, location);
            return{ location };
        }
    };

    inline std::shared_ptr<Texture> loadTextureFromBmp(RenderSystem& renderSystem, const std::string& path)
    {
        return renderSystem.createTexture(decodeBmpImage(path));
    }

    /** Sampler */
    class Sampler
    {
    private:
        D3D12_SAMPLER_DESC desc_;

    public:
        /** Resource view */
        struct View
        {
            D3D12_CPU_DESCRIPTOR_HANDLE d3dView;
        };

        /** Construct */
        Sampler()
            : desc_()
        {
            ZeroMemory(&desc_, sizeof(desc_));
            desc_.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            desc_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            desc_.MinLOD = -D3D12_FLOAT32_MAX;
            desc_.MaxLOD = D3D12_FLOAT32_MAX;
            desc_.MipLODBias = 0;
            desc_.MaxAnisotropy = 1;
            desc_.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        }

        /** Create the Direct3D view into a desctipror heap */
        View createD3DView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE location)
        {
            device->CreateSampler(&desc_, location);
            return{ location };
        }
    };
}

#endif