#ifndef _KILLME_MATERIAL_H_
#define _KILLME_MATERIAL_H_

#include "../resources/resource.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace killme
{
    class RenderSystem;
    class ResourceManager;
    class EffectTechnique;

    /** The material control the view of vertices */
    class Material : public IsResource
    {
    private:
        std::string useTech_;
        std::unordered_map<std::string, std::shared_ptr<EffectTechnique>> techMap_;

    public:
        /** Set  variable parameter */
        template <class T>
        void setVariable(const std::string& name, const T& value)
        {
            setVariableImpl(name, &value);
        }

        /** Add technique */
        void addTechnique(const std::string& name, const std::shared_ptr<EffectTechnique>& tech);

        /** Return current technique */
        std::shared_ptr<EffectTechnique> getUseTechnique();

        /** Change current technique */
        void selectTechnique(const std::string& name);

    private:
        void setVariableImpl(const std::string& name, const void* data);
    };

    std::shared_ptr<Material> loadMaterial(RenderSystem& renderSystem, ResourceManager& resourceManager, const std::string& path);
}

#endif