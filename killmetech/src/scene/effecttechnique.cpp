#include "effecttechnique.h"
#include "effectpass.h"
#include "materialcreation.h"

namespace killme
{
    EffectTechnique::EffectTechnique(const std::shared_ptr<RenderSystem>& renderSystem, ResourceManager& resourceManager,
        const MaterialDescription& matDesc, const TechniqueDescription& techDesc)
        : passes_()
    {
        for (const auto& pass : techDesc.passes)
        {
            passes_.emplace_back(std::make_shared<EffectPass>(renderSystem, resourceManager, matDesc, pass.second));
        }
    }

    void EffectTechnique::updateConstant(const std::string& matParam, const void* data, size_t size)
    {
        for (const auto& pass : passes_)
        {
            pass->updateConstant(matParam, data, size);
        }
    }

    void EffectTechnique::updateTexture(const std::string& matParam, const Resource<Texture>& tex)
    {
        for (const auto& pass : passes_)
        {
            pass->updateTexture(matParam, tex);
        }
    }

    void EffectTechnique::updateSampler(const std::string& matParam, const std::shared_ptr<Sampler>& sam)
    {
        for (const auto& pass : passes_)
        {
            pass->updateSampler(matParam, sam);
        }
    }
}