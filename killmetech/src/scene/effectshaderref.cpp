#include "effectshaderref.h"
#include "../renderer/rendersystem.h"
#include "../renderer/constantbuffer.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include <cassert>

namespace killme
{
    EffectShaderRef::EffectShaderRef(RenderSystem& renderSystem, const Resource<VertexShader>& vs)
        : vertexShader_(vs)
        , pixelShader_()
        , constantsMap_()
        , constantsDesc_()
        , constantBuffer_()
    {
        const auto cbuffers = vertexShader_.access()->describeConstnatBuffers();
        assert(cbuffers.length() <= 1 && "Invalid vertex shader");
        if (cbuffers.length() == 1)
        {
            constantsDesc_ = *std::cbegin(cbuffers);
            initialize(renderSystem);
        }
    }

    EffectShaderRef::EffectShaderRef(RenderSystem& renderSystem, const Resource<PixelShader>& ps)
        : vertexShader_()
        , pixelShader_(ps)
        , constantsMap_()
        , constantsDesc_()
        , constantBuffer_()
    {
        const auto cbuffers = pixelShader_.access()->describeConstnatBuffers();
        assert(cbuffers.length() <= 1 && "Invalid pixel shader");
        if (cbuffers.length() == 1)
        {
            constantsDesc_ = *std::cbegin(cbuffers);
            initialize(renderSystem);
        }
    }

    void EffectShaderRef::updateConstant(const std::string& param, const void* data)
    {
        const auto it = constantsMap_.find(param);
        if (it == std::cend(constantsMap_))
        {
            return;
        }
        if (const auto var = (constantsDesc_ ? constantsDesc_->describeVariable(it->second) : nullopt))
        {
            const auto size = var->size;
            const auto offset = var->offset;
            constantBuffer_->update(data, offset, size);
        }
    }

    void EffectShaderRef::mapParameter(const std::string& param, const std::string& constant)
    {
        constantsMap_[param] = constant;
    }

    Resource<VertexShader> EffectShaderRef::getReferenceVertexShader()
    {
        return vertexShader_;
    }

    Resource<PixelShader> EffectShaderRef::getReferencePixelShader()
    {
        return pixelShader_;
    }

    void EffectShaderRef::initialize(RenderSystem& renderSystem)
    {
        constantBuffer_ = renderSystem.createConstantBuffer(constantsDesc_->getSize());
        for (const auto var : constantsDesc_->describeVariables())
        {
            if (var.second.defaultValue)
            {
                const auto p = var.second.defaultValue.get();
                constantBuffer_->update(p, var.second.offset, var.second.size);
            }
        }
    }
}