#ifndef _KILLME_EFFECTTECHNIQUE_H_
#define _KILLME_EFFECTTECHNIQUE_H_

#include "../resources/resource.h"
#include "../core/utility.h"
#include <memory>
#include <vector>
#include <utility>

namespace killme
{
    class EffectPass;
    class Texture;

    /** Effect technique */
    class EffectTechnique
    {
    private:
        std::vector<std::pair<int, std::shared_ptr<EffectPass>>> passes_;

    public:
        /** Update constant */
        void updateConstant(const std::string& param, const void* data);

        /** Update texture */
        void updateTexture(const std::string& param, const Resource<Texture>& tex);

        /** Add pass with ordered by index*/
        void addPass(int index, const std::shared_ptr<EffectPass>& pass);

        /** Return passes */
        auto getPasses()
            -> decltype(makeRange(passes_))
        {
            return makeRange(passes_);
        }
    };
}

#endif