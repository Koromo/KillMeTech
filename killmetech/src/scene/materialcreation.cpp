#include "materialcreation.h"
#include "material.h"
#include "../renderer/texture.h"
#include "../renderer/shaders.h"
#include "../resources/resource.h"
#include <fstream>
#include <iterator>
#include <queue>
#include <unordered_set>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <array>
#include <cctype>

namespace killme
{
    namespace
    {
        // Get boolean from string
        bool stob(const std::string& s)
        {
            if (s == "true")
            {
                return true;
            }
            else if (s == "false")
            {
                return false;
            }
            throw std::invalid_argument(s + "is not boolean.");
        }

        // Get LightIteration from string
        LightIteration toLightIteration(const std::string& s)
        {
            if (s == "none")
            {
                return LightIteration::none;
            }
            else if (s == "directional")
            {
                return  LightIteration::directional;
            }
            else if (s == "point")
            {
                return  LightIteration::point;
            }
            throw std::invalid_argument(s + "is not LightIteration.");
        }

        // Get Blend from string
        Blend toBlend(const std::string& s)
        {
            if (s == "one")
            {
                return Blend::one;
            }
            else if (s == "zero")
            {
                return  Blend::zero;
            }
            throw std::invalid_argument(s + "is not Blend.");
        }
        
        // Get BlendOp from string
        BlendOp toBlendOp(const std::string& s)
        {
            if (s == "add")
            {
                return BlendOp::add;
            }
            else if (s == "subtract")
            {
                return  BlendOp::subtract;
            }
            else if (s == "min")
            {
                return  BlendOp::min;
            }
            else if (s == "max")
            {
                return BlendOp::max;
            }
            throw std::invalid_argument(s + "is not BlendOp.");
        }

        // Context
        struct ParseContext
        {
            std::vector<std::string> tokens;
            std::queue<size_t> numTokensInLine;

            std::string path;
            size_t line; // Current line
            decltype(std::cbegin(std::vector<std::string>())) token; // Current token
            decltype(token) tokenEnd;

            MaterialDescription material;

            ShaderBoundDescription* currentShaderBound;
            PassDescription* currentPass;
            TechniqueDescription* currentTech;

            std::unordered_set<std::string> identifiers;

            ResourceManager* resourceManager;
        };

        // Parser map type
        using ParserMap = std::unordered_map<std::string, std::function<void(ParseContext&)>>;

        // Throw error
        void error(const ParseContext& context, const std::string& msg)
        {
            throw MaterialLoadException(
                msg + "\n" +
                "FILE: " + context.path + "\n" +
                "LINE: " + std::to_string(context.line));
        }

        // Throw Syntax error message
        void syntaxError(const ParseContext& context, const std::string& syntax)
        {
            error(context, "Syntax error : Unexpected token \'" + syntax + "\'.");
        }

        // Check current token
        void check(const ParseContext& context, const std::string& expect)
        {
            if (*context.token != expect)
            {
                syntaxError(context, *context.token);
            }
        }

        // Forward current token
        std::string forward(ParseContext& context, const std::string& except = "")
        {
            enforce<MaterialLoadException>(context.token != context.tokenEnd, "Unexpected EOF.");
            ++context.token;
            --context.numTokensInLine.front();
            while (context.numTokensInLine.front() == 0)
            {
                context.numTokensInLine.pop();
                ++context.line;
            }

            if (!except.empty())
            {
                check(context, except);
            }

            return *context.token;
        }

        // Read string literal
        std::string literal_string(ParseContext& context)
        {
            forward(context, "\"");
            const auto str = forward(context);
            forward(context, "\"");
            return str;
        }

        // Read array literal
        template <class T, size_t L>
        std::array<T, L> literal_array(ParseContext& context)
        {
            forward(context, "[");

            std::array<T, L> arr;
            for (size_t i = 0; i < L - 1; ++i)
            {
                try
                {
                    arr[i] = std::stof(forward(context));
                }
                catch (const std::invalid_argument&)
                {
                    syntaxError(context, *context.token);
                }
                forward(context, ",");
            }

            try
            {
                arr[L - 1] = std::stof(forward(context));
            }
            catch (const std::invalid_argument&)
            {
                syntaxError(context, *context.token);
            }

            forward(context, "]");

            return arr;
        }

        // Parse current token
        void parseCurrentToken(ParseContext& context, const ParserMap& map)
        {
            const auto it = map.find(*context.token);
            if (it == std::cend(map))
            {
                syntaxError(context, *context.token);
            }
            it->second(context);
        }

        // Check identifier
        void addIdentifier(ParseContext& context, const std::string& name)
        {
            const auto added = context.identifiers.emplace(name).second;
            if (!added)
            {
                error(context, "Itentifier \'" + name + "\' is already exists.");
            }
        }

        // Check numeric material parameter
        void checkNumeric(const ParseContext& context, const std::string& name)
        {
            if (const auto type = context.material.getParameterType(name))
            {
                if (!isNumeric(*type))
                {
                    error(context, "Parameter \'" + name + "\' is not numeric.");
                }
                return;
            }
            else
            {
                error(context, "Parameter \'" + name + "\' is not exists.");
            }
        }

        // Check texture material parameter
        void checkTexture(const ParseContext& context, const std::string& name)
        {
            if (const auto type = context.material.getParameterType(name))
            {
                if (!isTexture(*type))
                {
                    error(context, "Parameter \'" + name + "\' is not texture.");
                }
            }
            else
            {
                error(context, "Parameter \'" + name + "\' is not exists.");
            }
        }

        // Check shader bound
        void checkShaderBound(const ParseContext& context, ShaderType type, const std::string& name)
        {
            if (!context.material.hasShaderBound(type, name))
            {
                if (type == ShaderType::vertex)
                {
                    error(context, "Vertex shader bound \'" + name + "\' is not exists.");
                }
                else if (type == ShaderType::pixel)
                {
                    error(context, "Pixel shader bound \'" + name + "\' is not exists.");
                }
                else if (type == ShaderType::geometry)
                {
                    error(context, "Geometry shader bound \'" + name + "\' is not exists.");
                }
            }
        }

        // Check whether pass is invalid or not
        void checkPass(const ParseContext& context, const PassDescription& pass)
        {
            if (pass.shaderRef.find(ShaderType::vertex) == std::cend(pass.shaderRef) ||
                pass.shaderRef.find(ShaderType::pixel) == std::cend(pass.shaderRef))
            {
                error(context, "\'vertex_shader_ref\' or \'pixel_shader_ref\' is not found in pass.");
            }
        }

        // Check pass index
        void checkPassIndex(const ParseContext& context, const TechniqueDescription& tech, size_t index)
        {
            if (tech.passes.find(index) != std::cend(tech.passes))
            {
                error(context, "Pass index " + std::to_string(index) + " is already exists.");
            }
        }

        // Parse "float", "float3" or float4 elements
        template <class Float, size_t N>
        void elem_float(ParseContext& context)
        {
            const auto name = forward(context);

            addIdentifier(context, name);

            forward(context);

            Float value;
            if (*context.token == "=")
            {
                const auto arr = literal_array<float, N>(context);
                forward(context, ";");

                for (int i = 0; i < N; ++i)
                {
                    value[i] = arr[i];
                }
            }
            else if (*context.token == ";")
            {
                value = Float::INIT;
            }
            else
            {
                syntaxError(context, *context.token);
            }

            forward(context);

            context.material.addParameter(name, { typeNumber<Float>(), Variant(value) });
        }

        // Parse "tex2d" element
        void elem_tex2d(ParseContext& context)
        {
            const auto name = forward(context);

            addIdentifier(context, name);

            forward(context);

            MP_tex2d value;
            if (*context.token == "=")
            {
                const auto path = literal_string(context);
                forward(context, ";");

                value = MP_tex2d::INIT;
                value.texture = Resource<Texture>(*context.resourceManager, path);
            }
            else if (*context.token == ";")
            {
                value = MP_tex2d::INIT;
            }
            else
            {
                syntaxError(context, *context.token);
            }

            forward(context);

            context.material.addParameter(name, { typeNumber<MP_tex2d>(), Variant(value) });
        }

        // Parse "parameters" block
        void block_parameters(ParseContext& context)
        {
            static ParserMap map({
                { "float", elem_float<MP_float, 1> },
                { "float3", elem_float<MP_float3, 3> },
                { "float4", elem_float<MP_float4, 4> },
                { "tex2d", elem_tex2d }
            });

            forward(context, "{");
            forward(context);

            while (*context.token != "}")
            {
                parseCurrentToken(context, map);
            }

            forward(context);
        }

        // Parse "source" element
        void elem_source(ParseContext& context)
        {
            forward(context, "=");
            const auto path = literal_string(context);
            forward(context, ";");
            forward(context);

            context.currentShaderBound->path = path;
        }

        // Parse "map_to_constant" element
        void elem_map_to_constant(ParseContext& context)
        {
            forward(context, "(");
            const auto paramName = forward(context);

            checkNumeric(context, paramName);

            forward(context, ",");
            const auto constantName = literal_string(context);
            forward(context, ")");
            forward(context, ";");
            forward(context);

            context.currentShaderBound->constantMapping[constantName] = paramName;
        }

        // Parse "map_to_texture" element
        void elem_map_to_texture(ParseContext& context)
        {
            forward(context, "(");
            const auto paramName = forward(context);

            checkTexture(context, paramName);

            forward(context, ",");
            const auto textureName = literal_string(context);
            forward(context, ",");
            const auto samplerName = literal_string(context);
            forward(context, ")");
            forward(context, ";");
            forward(context);

            context.currentShaderBound->textureMapping[textureName] = paramName;
            context.currentShaderBound->samplerMapping[samplerName] = paramName;
        }

        // Parse "vertex_shader", "pixel_shader" or "geometry_shader" block
        template <ShaderType Type>
        void block_shader(ParseContext& context)
        {
            static ParserMap map({
                { "source", elem_source },
                { "map_to_constant", elem_map_to_constant },
                { "map_to_texture", elem_map_to_texture }
            });

            ShaderBoundDescription shaderBound;
            context.currentShaderBound = &shaderBound;

            const auto name = forward(context);

            addIdentifier(context, name);

            forward(context, "{");
            forward(context);

            while (*context.token != "}")
            {
                parseCurrentToken(context, map);
            }

            forward(context);

            context.currentShaderBound = nullptr;
            context.material.addShaderBound(Type, name, std::move(shaderBound));
        }

        // Parse "blend_enable" element
        void elem_blend_enable(ParseContext& context)
        {
            forward(context, "=");

            bool enable;
            try
            {
                enable = stob(forward(context));
            }
            catch (const std::invalid_argument&)
            {
                syntaxError(context, *context.token);
            }

            forward(context, ";");
            forward(context);

            context.currentPass->blendState.enable = enable;
        }

        // Parse "blend_src" element
        void elem_blend_src(ParseContext& context)
        {
            forward(context, "=");

            Blend blend;
            try
            {
                blend = toBlend(forward(context));
            }
            catch (const std::invalid_argument&)
            {
                syntaxError(context, *context.token);
            }

            forward(context, ";");
            forward(context);

            context.currentPass->blendState.src = blend;
        }

        // Parse "blend_dest" element
        void elem_blend_dest(ParseContext& context)
        {
            forward(context, "=");

            Blend blend;
            try
            {
                blend = toBlend(forward(context));
            }
            catch (const std::invalid_argument&)
            {
                syntaxError(context, *context.token);
            }

            forward(context, ";");
            forward(context);

            context.currentPass->blendState.dest = blend;
        }

        // Parse "blend_op" element
        void elem_blend_op(ParseContext& context)
        {
            forward(context, "=");

            BlendOp op;
            try
            {
                op = toBlendOp(forward(context));
            }
            catch (const std::invalid_argument&)
            {
                syntaxError(context, *context.token);
            }

            forward(context, ";");
            forward(context);

            context.currentPass->blendState.op = op;
        }

        // Parse "for_each_light" element
        void elem_for_each_light(ParseContext& context)
        {
            forward(context, "=");

            LightIteration type;
            try
            {
                type = toLightIteration(forward(context));
            }
            catch (const std::invalid_argument&)
            {
                syntaxError(context, *context.token);
            }

            forward(context, ";");
            forward(context);

            context.currentPass->lightIteration = type;
        }

        // Parse "vertex_shader_ref", "pixel_shader_ref" or "geometry_shader_ref", element
        template <ShaderType Type>
        void elem_shader_ref(ParseContext& context)
        {
            forward(context, "=");
            const auto name = forward(context);

            checkShaderBound(context, Type, name);

            forward(context, ";");
            forward(context);

            context.currentPass->shaderRef[Type] = name;
        }

        // Parse "pass" block
        void block_pass(ParseContext& context)
        {
            static ParserMap map({
                { "vertex_shader_ref", elem_shader_ref<ShaderType::vertex> },
                { "pixel_shader_ref", elem_shader_ref<ShaderType::pixel> },
                { "geometry_shader_ref", elem_shader_ref<ShaderType::geometry> },
                { "for_each_light", elem_for_each_light },
                { "blend_op", elem_blend_op },
                { "blend_enable", elem_blend_enable },
                { "blend_src", elem_blend_src },
                { "blend_dest", elem_blend_dest }
            });

            PassDescription pass;
            pass.lightIteration = LightIteration::none;
            pass.blendState = BlendState::DEFAULT;
            context.currentPass = &pass;

            size_t index;
            try
            {
                index = std::stoi(forward(context));
            }
            catch (const std::invalid_argument&)
            {
                syntaxError(context, *context.token);
            }

            checkPassIndex(context, *context.currentTech, index);

            forward(context, "{");
            forward(context);

            while (*context.token != "}")
            {
                parseCurrentToken(context, map);
            }

            checkPass(context, pass);

            forward(context);

            context.currentPass = nullptr;
            context.currentTech->passes.emplace(index, pass);
        }

        // Parse "technique" block
        void block_technique(ParseContext& context)
        {
            static ParserMap map({
                { "pass", block_pass }
            });

            TechniqueDescription tech;
            context.currentTech = &tech;

            const auto name = forward(context);

            addIdentifier(context, name);

            forward(context, "{");
            forward(context);

            while (*context.token != "}")
            {
                parseCurrentToken(context, map);
            }

            forward(context);

            context.currentTech = nullptr;
            context.material.addTechnique(name, std::move(tech));
        }

        // Initialize context
        void initContext(ParseContext& context)
        {
            addIdentifier(context, "true");
            addIdentifier(context, "false");
            addIdentifier(context, "one");
            addIdentifier(context, "zero");
            addIdentifier(context, "add");
            addIdentifier(context, "subtract");
            addIdentifier(context, "min");
            addIdentifier(context, "max");
            addIdentifier(context, "parameters");
            addIdentifier(context, "float");
            addIdentifier(context, "float3");
            addIdentifier(context, "float4");
            addIdentifier(context, "tex2d");
            addIdentifier(context, "vertex_shader");
            addIdentifier(context, "pixel_shader");
            addIdentifier(context, "geometry_shader");
            addIdentifier(context, "source");
            addIdentifier(context, "map_to_constant");
            addIdentifier(context, "map_to_texture");
            addIdentifier(context, "technique");
            addIdentifier(context, "pass");
            addIdentifier(context, "vertex_shader_ref");
            addIdentifier(context, "pixel_shader_ref");
            addIdentifier(context, "geometry_shader_ref");
            addIdentifier(context, "for_each_light");
            addIdentifier(context, "none");
            addIdentifier(context, "directional");
            addIdentifier(context, "point");
            addIdentifier(context, "blend_enable");
            addIdentifier(context, "blend_op");
            addIdentifier(context, "blend_src");
            addIdentifier(context, "blend_dest");

            addIdentifier(context, "_WorldMatrix");
            addIdentifier(context, "_ViewMatrix");
            addIdentifier(context, "_ProjMatrix");
            addIdentifier(context, "_AmbientLight");
            addIdentifier(context, "_LightColor");
            addIdentifier(context, "_LightDirection");
            addIdentifier(context, "_LightPosition");
            addIdentifier(context, "_LightAttRange");
            addIdentifier(context, "_LightAttConstant");
            addIdentifier(context, "_LightAttLiner");
            addIdentifier(context, "_LightAttQuadratic");

            context.material.addParameter("_WorldMatrix", { typeNumber<MP_float4x4>(), Variant(MP_float4x4::INIT) });
            context.material.addParameter("_ViewMatrix", { typeNumber<MP_float4x4>(), Variant(MP_float4x4::INIT) });
            context.material.addParameter("_ProjMatrix", { typeNumber<MP_float4x4>(), Variant(MP_float4x4::INIT) });
            context.material.addParameter("_AmbientLight", { typeNumber<MP_float4>(), Variant(MP_float4::INIT) });
            context.material.addParameter("_LightColor", { typeNumber<MP_float4>(), Variant(MP_float4::INIT) });
            context.material.addParameter("_LightDirection", { typeNumber<MP_float3>(), Variant(MP_float3::INIT) });
            context.material.addParameter("_LightPosition", { typeNumber<MP_float3>(), Variant(MP_float3::INIT) });
            context.material.addParameter("_LightAttRange", { typeNumber<MP_float>(), Variant(MP_float::INIT) });
            context.material.addParameter("_LightAttConstant", { typeNumber<MP_float>(), Variant(MP_float::INIT) });
            context.material.addParameter("_LightAttLiner", { typeNumber<MP_float>(), Variant(MP_float::INIT) });
            context.material.addParameter("_LightAttQuadratic", { typeNumber<MP_float>(), Variant(MP_float::INIT) });
        }

        // Step position by find nonspace character
        size_t stepByNonspace(size_t p, const std::string& str)
        {
            while (p < str.length() && std::isspace(str[p]))
            {
                ++p;
            }
            return p;
        }

        // Tokenize line by delimiters
        std::vector<std::string> tokenizeLine(const std::string& line)
        {
            static const std::string DERIMITERS = " \t\",=(){}[];";

            std::vector<std::string> tokens;

            for (auto p = stepByNonspace(0, line); p < line.length(); p = stepByNonspace(p, line))
            {
                auto q = line.find_first_of(DERIMITERS, p);
                if (q == std::string::npos)
                {
                    q = line.length();
                }

                if (p == q)
                {
                    tokens.emplace_back(line.substr(p, 1));
                    p = q + 1;
                }
                else
                {
                    tokens.emplace_back(line.substr(p, q - p));
                    p = q;
                }
            }

            return tokens;
        }

        void tokenize(std::ifstream& stream, ParseContext& context)
        {
            bool commentOut = false;

            while (!stream.eof())
            {
                // Get current line
                std::string line;
                std::getline(stream, line);

                // Erase comments
                if (commentOut)
                {
                    const auto p = line.find("*/");
                    if (p == std::string::npos)
                    {
                        line.clear();
                    }
                    else
                    {
                        line = line.substr(p + 2);
                        commentOut = false;
                    }
                }

                const auto p = line.find("/*");
                if (p != std::string::npos)
                {
                    line = line.substr(0, p);
                    commentOut = true;
                }

                line = line.substr(0, line.find("//"));

                // Tokelize line
                const auto lineTokens = tokenizeLine(line);
                context.tokens.insert(std::cend(context.tokens), std::begin(lineTokens), std::cend(lineTokens));
                context.numTokensInLine.push(lineTokens.size());
            }
        }
    }

    void MaterialDescription::addParameter(const std::string& name, MaterialParameterDescription&& desc)
    {
        paramMap_.emplace(name, std::move(desc));
    }

    void MaterialDescription::addShaderBound(ShaderType type, const std::string& name, ShaderBoundDescription&& desc)
    {
        shaderBoundMap_[type].emplace(name, std::move(desc));
    }

    void MaterialDescription::addTechnique(const std::string& name, TechniqueDescription&& desc)
    {
        techs_.emplace_back(std::make_pair(name, std::move(desc)));
    }

    Optional<TypeNumber> MaterialDescription::getParameterType(const std::string& name) const
    {
        const auto it = paramMap_.find(name);
        if (it != std::cend(paramMap_))
        {
            return it->second.type;
        }
        return nullopt;
    }

    bool MaterialDescription::hasShaderBound(ShaderType type, const std::string& name) const
    {
        return const_cast<MaterialDescription&>(*this).shaderBoundMap_[type].find(name)
            != std::cend(const_cast<MaterialDescription&>(*this).shaderBoundMap_[type]);
    }

    const ShaderBoundDescription& MaterialDescription::getShaderBound(ShaderType type, const std::string& name) const
    {
        return const_cast<MaterialDescription&>(*this).shaderBoundMap_[type][name];
    }

    MaterialLoadException::MaterialLoadException(const std::string& msg)
        : FileException(msg)
    {
    }

    std::shared_ptr<Material> loadMaterial(RenderSystem& renderSystem, ResourceManager& resourceManager, const std::string& path)
    {
        std::ifstream stream(path);
        enforce<MaterialLoadException>(stream.is_open(), "Failed to open file (" + path + ").");

        ParseContext context;
        context.path = path;
        context.line = 0;
        context.currentShaderBound = nullptr;
        context.currentPass = nullptr;
        context.currentTech = nullptr;
        context.resourceManager = &resourceManager;

        context.tokens.emplace_back("MATERIAL_BEGIN");
        context.numTokensInLine.push(1);

        // Tokenize source code
        tokenize(stream, context);
        stream.close();

        context.tokens.emplace_back("MATERIAL_END");
        context.numTokensInLine.push(1);
        context.token = std::cbegin(context.tokens);
        context.tokenEnd = std::cend(context.tokens);

        // Parse
        static ParserMap map({
            { "parameters", block_parameters },
            { "vertex_shader", block_shader<ShaderType::vertex> },
            { "pixel_shader", block_shader<ShaderType::pixel> },
            { "geometry_shader", block_shader<ShaderType::geometry> },
            { "technique", block_technique }
        });

        initContext(context);

        forward(context);
        while (*context.token != "MATERIAL_END")
        {
            parseCurrentToken(context, map);
        }

        return std::make_shared<Material>(renderSystem, resourceManager, context.material);
    }
}