#ifndef _KILLME_EFFECTSHADERREF_H_
#define _KILLME_EFFECTSHADERREF_H_

#include "../renderer/shader.h"
#include "../resources/resource.h"
#include "../core/optional.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>

namespace killme
{
    class RenderSystem;
    class ConstantBuffer;
    class VertexShader;
    class PixelShader;

    /** Effect shader reference */
    class EffectShaderRef
    {
    private:
        Resource<VertexShader> vertexShader_;
        Resource<PixelShader> pixelShader_;
        std::unordered_map<std::string, std::string> constantsMap_;
        Optional<ConstantBufferDescription> constantsDesc_;
        std::shared_ptr<ConstantBuffer> constantBuffer_;

    public:
        /** Construct as the vertex shader reference */
        EffectShaderRef(RenderSystem& renderSystem, const Resource<VertexShader>& vs);

        /** Construct as the pixel shader reference */
        EffectShaderRef(RenderSystem& renderSystem, const Resource<PixelShader>& ps);

        /** Update constant */
        void updateConstant(const std::string& param, const void* data);

        /** Map parameter */
        void mapParameter(const std::string& param, const std::string& constant);

        /** Return constant buffer description */
        auto describeConstantBuffer()
            -> decltype(std::make_pair(constantsDesc_, constantBuffer_))
        {
            return std::make_pair(constantsDesc_, constantBuffer_);
        }

        /** Return reference shader */
        Resource<VertexShader> getReferenceVertexShader();

        /** ditto */
        Resource<PixelShader> getReferencePixelShader();

    private:
        void initialize(RenderSystem& renderSystem);
    };
}

#endif