#include "effectshaderref.h"
#include "../core/platform.h"
#include <cassert>

namespace killme
{
    EffectShaderRef::EffectShaderRef(const Resource<VertexShader>& vs)
        : type_(ShaderType::vertex)
        , vertexShader_(vs)
        , pixelShader_()
        , cbufferDesc_()
        , constantMap_()
        , textureMap_()
        , samplerMap_()
    {
        const auto cbuffers = vertexShader_.access()->describeConstnatBuffers();
        assert(cbuffers.length() <= 1 && "Invalid vertex shader");
        if (cbuffers.length() == 1)
        {
            cbufferDesc_ = *std::cbegin(cbuffers);
        }
    }

    EffectShaderRef::EffectShaderRef(const Resource<PixelShader>& ps)
        : type_(ShaderType::pixel)
        , vertexShader_()
        , pixelShader_(ps)
        , cbufferDesc_()
        , constantMap_()
        , textureMap_()
    {
        const auto cbuffers = pixelShader_.access()->describeConstnatBuffers();
        assert(cbuffers.length() <= 1 && "Invalid pixel shader");
        if (cbuffers.length() == 1)
        {
            cbufferDesc_ = *std::cbegin(cbuffers);
        }
    }

    ShaderType EffectShaderRef::getType() const
    {
        return type_;
    }

    Optional<ConstantBufferDescription> EffectShaderRef::describeConstantBuffer()
    {
        return cbufferDesc_;
    }

    void EffectShaderRef::mapToConstant(const std::string& matParam, const std::string& shaderConstant)
    {
        assert(cbufferDesc_ && cbufferDesc_->describeVariable(shaderConstant) && "Invalid constant mapping.");
        constantMap_.emplace(matParam, shaderConstant);
    }

    void EffectShaderRef::mapToTexture(const std::string& matParam, const std::string& shaderTexture, const std::string& shaderSampler)
    {
#ifdef KILLME_DEBUG
        if (type_ == ShaderType::vertex)
        {
            auto desc = vertexShader_.access()->describeBoundResource(shaderTexture);
            assert(desc && "Invalid texture mapping.");
            assert(desc->getType() == BoundResourceType::texture && "Invalid texture mapping.");
            desc = vertexShader_.access()->describeBoundResource(shaderSampler);
            assert(desc && "Invalid sampler mapping.");
            assert(desc->getType() == BoundResourceType::sampler && "Invalid sampler mapping.");
        }
        else
        {
            auto desc = pixelShader_.access()->describeBoundResource(shaderTexture);
            assert(desc && "Invalid texture mapping.");
            assert(desc->getType() == BoundResourceType::texture && "Invalid texture mapping.");
            desc = pixelShader_.access()->describeBoundResource(shaderSampler);
            assert(desc && "Invalid sampler mapping.");
            assert(desc->getType() == BoundResourceType::sampler && "Invalid sampler mapping.");
        }
#endif

        textureMap_.emplace(matParam, shaderTexture);
        samplerMap_.emplace(matParam, shaderSampler);
    }

    Resource<VertexShader> EffectShaderRef::getReferenceVertexShader()
    {
        return vertexShader_;
    }

    Resource<PixelShader> EffectShaderRef::getReferencePixelShader()
    {
        return pixelShader_;
    }
}