#include "effecttechnique.h"
#include "effectpass.h"
#include <algorithm>

namespace killme
{
    void EffectTechnique::updateConstant(const std::string& param, const void* data)
    {
        for (const auto& pass : passes_)
        {
            pass.second->updateConstant(param, data);
        }
    }

    void EffectTechnique::addPass(int index, const std::shared_ptr<EffectPass>& pass)
    {
        const auto point = std::lower_bound(std::cbegin(passes_), std::cend(passes_), index,
            [](const std::pair<int, std::shared_ptr<EffectPass>>& a, int i) { return a.first < i; });
        passes_.emplace(point, index, pass);
    }
}