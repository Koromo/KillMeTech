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
    class RenderDevice;
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
        EffectTechnique(RenderDevice& device, ResourceManager& resources,
            const MaterialDescription& matDesc, const TechniqueDescription& techDesc);

        /** Update constant */
        void updateConstant(const std::string& matParam, const void* data, size_t size);

        /** Update texture */
        void updateTexture(const std::string& matParam, const Resource<Texture>& tex);

        /** Update sampler */
        void updateSampler(const std::string& matParam, const std::shared_ptr<Sampler>& sam);

        /** Return passes */
        auto getPasses()
            -> decltype(constRange(passes_))
        {
            return constRange(passes_);
        }
    };
}

#endif