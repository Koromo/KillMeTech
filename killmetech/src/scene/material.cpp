#include "material.h"
#include "effecttechnique.h"
#include <cassert>

namespace killme
{
    void Material::addTechnique(const std::string& name, const std::shared_ptr<EffectTechnique>& tech)
    {
        const auto check = techMap_.emplace(name, tech);
        assert(check.second && "Conflict techniques");

        if (useTech_.empty())
        {
            useTech_ = name;
        }
    }

    std::shared_ptr<EffectTechnique> Material::getUseTechnique()
    {
        return techMap_[useTech_];
    }

    void Material::selectTechnique(const std::string& name)
    {
        useTech_ = name;
    }

    void Material::setVariableImpl(const std::string& name, const void* data)
    {
        for (const auto tech : techMap_)
        {
            tech.second->updateConstant(name, data);
        }
    }
}