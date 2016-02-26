#ifndef _KILLME_EFFECTSHADERREF_H_
#define _KILLME_EFFECTSHADERREF_H_

#include "../renderer/shader.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../resources/resource.h"
#include "../core/optional.h"
#include "../core/utility.h"
#include <unordered_map>
#include <utility>

namespace killme
{
    class VertexShader;
    class PixelShader;

    /** Effect shader reference */
    class EffectShaderRef
    {
    private:
        ShaderType type_;
        Resource<VertexShader> vertexShader_;
        Resource<PixelShader> pixelShader_;
        Optional<ConstantBufferDescription> cbufferDesc_;
        std::unordered_map<std::string, std::string> constantMap_;
        std::unordered_map<std::string, std::string> textureMap_;
        std::unordered_map<std::string, std::string> samplerMap_;

    public:
        /** Construct as the vertex shader reference */
        explicit EffectShaderRef(const Resource<VertexShader>& vs);

        /** Construct as the pixel shader reference */
        explicit EffectShaderRef(const Resource<PixelShader>& ps);

        /** Return shader type */
        ShaderType getType() const;

        /** Return constant buffer description */
        Optional<ConstantBufferDescription> describeConstantBuffer();

        /** Return bound resource descriptions */
        auto describeBoundResources(BoundResourceType type)
            -> decltype(vertexShader_.access()->describeBoundResources(type))
        {
            if (type_ == ShaderType::vertex)
            {
                return vertexShader_.access()->describeBoundResources(type);
            }
            return pixelShader_.access()->describeBoundResources(type);
        }

        /** Return constant mappings */
        auto getConstantMap() const
            -> decltype(makeRange(constantMap_))
        {
            return makeRange(constantMap_);
        }

        /** Return texture mappings */
        auto getTextureMap() const
            -> decltype(makeRange(textureMap_))
        {
            return makeRange(textureMap_);
        }

        /** Return sampler mappings */
        auto getSamplerMap() const
            -> decltype(makeRange(samplerMap_))
        {
            return makeRange(samplerMap_);
        }

        /** Map material parameter to shader constant */
        void mapToConstant(const std::string& matParam, const std::string& shaderConstant);

        /** Map material parameter to shader texture */
        void mapToTexture(const std::string& matParam, const std::string& shaderTexture, const std::string& shaderSampler);

        /** Return reference shader */
        Resource<VertexShader> getReferenceVertexShader();
        Resource<PixelShader> getReferencePixelShader();
    };
}

#endif