#ifndef _KILLME_EFFECTTECHNIQUE_H_
#define _KILLME_EFFECTTECHNIQUE_H_

#include "../resources/resource.h"
#include "../core/utility.h"
#include <memory>
#include <vector>
#include <string>

namespace killme
{
    class MaterialDescription;
    struct TechniqueDescription;
    class RenderSystem;
    class ResourceManager;
    class EffectPass;
    class Texture;
    class Sampler;

    /** Effect technique */
    class EffectTechnique
    {
    private:
        std::vector<std::shared_ptr<EffectPass>> passes_;

    public:
        /** Construct */
        EffectTechnique(const std::shared_ptr<RenderSystem>& renderSystem, ResourceManager& resourceManager,
            const MaterialDescription& matDesc, const TechniqueDescription& techDesc);

        /** Update constant */
        void updateConstant(const std::string& matParam, const void* data, size_t size);

        /** Update texture */
        void updateTexture(const std::string& matParam, const Resource<Texture>& tex);

        /** Update sampler */
        void updateSampler(const std::string& matParam, const std::shared_ptr<Sampler>& sam);

        /** Return passes */
        auto getPasses()
            -> decltype(makeRange(passes_))
        {
            return makeRange(passes_);
        }
    };
}

#endif