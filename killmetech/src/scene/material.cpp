#include "material.h"
#include "materialcreation.h"
#include "../renderer/texture.h"
#include <cassert>

namespace killme
{
    const MP_float MP_float::INIT = { 0 };
    const MP_float3 MP_float3::INIT = { 0, 0, 0 };
    const MP_float4 MP_float4::INIT = { 0, 0, 0, 0 };
    const MP_float4x4 MP_float4x4::INIT = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };
    const MP_tex2d MP_tex2d::INIT = { Resource<Texture>(), std::make_shared<Sampler>() };

    const float& MP_float::operator [](size_t i) const
    {
        switch (i)
        {
        case 0: return x;
        default:
            assert(false && "Index out of range.");
            return x; // For warning
        }
    }

    float& MP_float::operator [](size_t i)
    {
        return const_cast<float&>(static_cast<const MP_float&>(*this)[i]);
    }

    const float& MP_float3::operator [](size_t i) const
    {
        switch (i)
        {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default:
            assert(false && "Index out of range.");
            return x; // For warning
        }
    }

    float& MP_float3::operator [](size_t i)
    {
        return const_cast<float&>(static_cast<const MP_float3&>(*this)[i]);
    }

    const float& MP_float4::operator [](size_t i) const
    {
        switch (i)
        {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
        default:
            assert(false && "Index out of range.");
            return x; // For warning
        }
    }

    float& MP_float4::operator [](size_t i)
    {
        return const_cast<float&>(static_cast<const MP_float4&>(*this)[i]);
    }

    MP_float4x4::MP_float4x4(std::initializer_list<float> il)
        : m_()
    {
        *this = il;
    }

    MP_float4x4& MP_float4x4::operator =(std::initializer_list<float> il)
    {
        assert(il.size() == 16 && "Invalid initializer list.");

        auto it = std::begin(il);
        for (int r = 0; r < 4; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                m_[r][c] = *it;
                ++it;
            }
        }
        return *this;
    }

    const float& MP_float4x4::operator ()(size_t r, size_t c) const
    {
        assert(r < 4 && c < 4 && "Index out of range.");
        return m_[r][c];
    }

    float& MP_float4x4::operator ()(size_t r, size_t c)
    {
        return const_cast<float&>(static_cast<const MP_float4x4&>(*this)(r, c));
    }

    bool isNumeric(TypeNumber type)
    {
        return !isTexture(type);
    }

    bool isTexture(TypeNumber type)
    {
        return type == typeNumber<MP_tex2d>();
    }

    Material::Material(RenderSystem& renderSystem, ResourceManager& resourceManager, const MaterialDescription& desc)
        : params_()
        , useTech_()
        , techMap_()
    {
        for (const auto& tech : desc.getTechniques())
        {
            techMap_.emplace(tech.first, std::make_shared<EffectTechnique>(renderSystem, resourceManager, desc, tech.second));
            if (useTech_.empty())
            {
                useTech_ = tech.first;
            }
        }

        for (const auto& paramDesc : desc.getParameters())
        {
            const auto name = paramDesc.first;
            Param param;
            param.type = paramDesc.second.type;
            param.value = paramDesc.second.value;
            params_.emplace(name, param);

            if (isNumeric(param.type))
            {
                for (const auto& tech : techMap_)
                {
                    tech.second->updateConstant(name, param.value.ptr(), param.value.sizeOf());
                }
            }
            else
            {
                const auto tex = to<MP_tex2d>(param.value);
                for (const auto& tech : techMap_)
                {
                    if (tex.texture.bound())
                    {
                        tech.second->updateTexture(name, tex.texture);
                    }
                    tech.second->updateSampler(name, tex.sampler);
                }
            }
        }
    }

    std::shared_ptr<EffectTechnique> Material::getUseTechnique()
    {
        return techMap_[useTech_];
    }

    void Material::selectTechnique(const std::string& name)
    {
        enforce<ItemNotFoundException>(techMap_.find(name) != std::cend(techMap_), "Technique \'" + name + "\' not found.");
        useTech_ = name;
    }

    void Material::setTexture(const std::string& name, const Resource<Texture>& tex)
    {
        assert(tex.bound() && "No texture.");

        const auto it = params_.find(name);
        if (it != std::cend(params_))
        {
            enforce<InvalidArgmentException>(it->second.type == typeNumber<MP_tex2d>(), "Mismatch texture parameter type.");
            to<MP_tex2d&>(it->second.value).texture = tex;
            for (const auto& tech : techMap_)
            {
                tech.second->updateTexture(name, tex);
            }
        }
    }

    void Material::setTexture(const std::string& name, const MP_tex2d& tex)
    {
        setTexture(name, tex.texture);
        setSampler(name, tex.sampler);
    }

    void Material::setSampler(const std::string& name, const std::shared_ptr<Sampler>& sam)
    {
        assert(sam && "No sampler.");

        const auto it = params_.find(name);
        if (it != std::cend(params_))
        {
            enforce<InvalidArgmentException>(it->second.type == typeNumber<MP_tex2d>(), "Mismatch texture parameter type.");
            to<MP_tex2d&>(it->second.value).sampler = sam;
            for (const auto& tech : techMap_)
            {
                tech.second->updateSampler(name, sam);
            }
        }
    }
}