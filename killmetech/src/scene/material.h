#ifndef _KILLME_MATERIAL_H_
#define _KILLME_MATERIAL_H_

#include "effecttechnique.h"
#include "../resources/resource.h"
#include "../core/math/vector3.h"
#include "../core/math/matrix44.h"
#include "../core/math/color.h"
#include "../core/utility.h"
#include "../core/variant.h"
#include "../core/exception.h"
#include <array>
#include <memory>
#include <string>
#include <unordered_map>

namespace killme
{
    class MaterialDescription;
    class RenderSystem;
    class ResourceManager;
    class Texture;
    class Sampler;
    class EffectTechnique;

    /** float parameter */
    struct MP_float
    {
        float x;
        const float& operator [](size_t i) const;
        float& operator [](size_t i);
        static const MP_float INIT;
    };

    /** float converters */
    template <>
    inline MP_float to<MP_float, float>(const float& f)
    {
        return{ f };
    }

    template <>
    inline float to<float, MP_float>(const MP_float& f)
    {
        return f.x;
    }

    /** float3 parameter */
    struct MP_float3
    {
        float x, y, z;

        const float& operator [](size_t i) const;
        float& operator [](size_t i);

        static const MP_float3 INIT;
    };

    /** float3 converters */
    template <>
    inline MP_float3 to<MP_float3, Vector3>(const Vector3& v)
    {
        return{ v.x, v.y, v.z };
    }

    template <>
    inline Vector3 to<Vector3, MP_float3>(const MP_float3& f)
    {
        return{ f.x, f.y, f.z };
    }

    /** float4 parameter */
    struct MP_float4
    {
        float x, y, z, w;

        const float& operator [](size_t i) const;
        float& operator [](size_t i);

        static const MP_float4 INIT;
    };

    /** float4 converters */
    template <>
    inline MP_float4 to<MP_float4, Color>(const Color& c)
    {
        return{ c.r, c.g, c.b, c.a };
    }

    template <>
    inline Color to<Color, MP_float4>(const MP_float4& f)
    {
        return{ f.x, f.y, f.z, f.w };
    }

    /** float4x4 parameter */
    struct MP_float4x4
    {
    private:
        std::array<std::array<float, 4>, 4> m_;

    public:
        MP_float4x4() = default;

        MP_float4x4(std::initializer_list<float> il);

        MP_float4x4(const MP_float4x4&) = default;
        MP_float4x4(MP_float4x4&&) = default;

        MP_float4x4& operator =(std::initializer_list<float> il);

        MP_float4x4& operator =(const MP_float4x4&) = default;
        MP_float4x4& operator =(MP_float4x4&&) = default;

        const float& operator ()(size_t r, size_t c) const;
        float& operator ()(size_t r, size_t c);

        static const MP_float4x4 INIT;
    };

    /** float4x4 converters */
    template <>
    inline MP_float4x4 to<MP_float4x4, Matrix44>(const Matrix44& m)
    {
        MP_float4x4 f;
        for (size_t r = 0; r < 4; ++r)
        {
            for (size_t c = 0; c < 4; ++c)
            {
                f(r, c) = m(r, c);
            }
        }
        return f;
    }

    template <>
    inline Matrix44 to<Matrix44, MP_float4x4>(const MP_float4x4& f)
    {
        Matrix44 m;
        for (size_t r = 0; r < 4; ++r)
        {
            for (size_t c = 0; c < 4; ++c)
            {
                m(r, c) = f(r, c);
            }
        }
        return m;
    }

    /** tex2d parameter */
    struct MP_tex2d
    {
        Resource<Texture> texture;
        std::shared_ptr<Sampler> sampler;

        static const MP_tex2d INIT;
    };

    /** Return whether parameter type is numeric or not */
    bool isNumeric(TypeNumber type);

    /** Return whether parameter type is texture or not */
    bool isTexture(TypeNumber type);

    /** Define the view of vertices */
    /// NOTE: See a note about the mesh in mesh.h
    class Material : public IsResource
    {
    private:
        struct Param
        {
            TypeNumber type;
            Variant value;
        };

        std::unordered_map<std::string, Param> params_;
        std::string useTech_;
        std::unordered_map<std::string, std::shared_ptr<EffectTechnique>> techMap_;

    public:
        /** Construct */
        Material(const std::shared_ptr<RenderSystem>& renderSystem, ResourceManager& resourceManager, const MaterialDescription& desc);

        /** Return current technique */
        std::shared_ptr<EffectTechnique> getUseTechnique();

        /** Change current technique */
        void selectTechnique(const std::string& name);

        /** Get parameter */
        template <class T>
        T getParameter(const std::string& name)
        {
            const auto it = params_.find(name);

            enforce<InvalidArgmentException>(it != std::cend(params_), "Parameter \'" + name + "\' not exists.");
            enforce<InvalidArgmentException>(it->second.type == typeNumber<T>(), "Mismatch parameter type.");

            return it->second.value;
        }

        /** Set numeric parameter */
        template <class T>
        void setNumeric(const std::string& name, const T& value)
        {
            const auto it = params_.find(name);
            if (it != std::cend(params_))
            {
                enforce<InvalidArgmentException>(it->second.type == typeNumber<T>(), "Mismatch numeric parameter type.");
                it->second.value = value;
                for (const auto& tech : techMap_)
                {
                    tech.second->updateConstant(name, &value, sizeof(value));
                }
            }
        }

        /** Set texture parameter */
        void setTexture(const std::string& name, const Resource<Texture>& tex);
        void setTexture(const std::string& name, const MP_tex2d& tex);
        void setSampler(const std::string& name, const std::shared_ptr<Sampler>& sam);
    };
}

#endif