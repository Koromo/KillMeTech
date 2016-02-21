#include "material.h"
#include "effecttechnique.h"
#include "effectpass.h"
#include "effectshaderref.h"
#include "../renderer/rendersystem.h"
#include "../renderer/vertexshader.h"
#include "../renderer/pixelshader.h"
#include "../resources/resourcemanager.h"
#include "../core/tree.h"
#include "../core/exception.h"
#include <vector>
#include <fstream>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <utility>
#include <string>
#include <memory>
#include <cctype>
#include <cassert>

namespace killme
{
    namespace
    {
        struct ParseNode;
        struct RootParser;
        struct Block_parameters;
        struct Elem_float4;
        //struct Block_shader;
        struct Elem_compile;
        struct Elem_map;
        struct Block_technique;
        struct Block_pass;
        //struct Elem_shaderRef;

        // For parse .material
        struct ParseContext
        {
            decltype(std::cbegin(std::vector<std::string>())) currentToken;
            decltype(currentToken) tokenEnd;
        };

        // Build infomations for shader reference
        struct ShaderRefBuild
        {
            std::string path;
            std::unordered_map<std::string, std::string> mappings; // param -> constant

            // Make EffectShaderRef
            template <class Shader>
            std::shared_ptr<EffectShaderRef> make(RenderSystem& rs, ResourceManager& rm)
            {
                assert(!path.empty() && "Build shader ref error.");

                const auto ref = std::make_shared<EffectShaderRef>(rs, rm.getAccessor<Shader>(path, true));
                for (const auto pair : mappings)
                {
                    ref->mapParameter(pair.first, pair.second);
                }

                return ref;
            }
        };

        // Build infomations for pass
        struct PassBuild
        {
            std::shared_ptr<EffectShaderRef> vsRef;
            std::shared_ptr<EffectShaderRef> psRef;

            template <class Shader>
            std::shared_ptr<EffectShaderRef>& shaderRef();

            template <>
            std::shared_ptr<EffectShaderRef>& shaderRef<VertexShader>()
            {
                return vsRef;
            }

            template <>
            std::shared_ptr<EffectShaderRef>& shaderRef<PixelShader>()
            {
                return psRef;
            }

            // Make EffectPass
            std::shared_ptr<EffectPass> make(RenderSystem& rs)
            {
                assert((vsRef && psRef) && "Build pass error.");
                return std::make_shared<EffectPass>(rs, vsRef, psRef);
            }
        };

        // For create material by parsed .material file
        struct BuildContext
        {
            std::shared_ptr<Material> material;
            std::unordered_set<std::string> params; // Parameters
            std::unordered_map<std::string, ShaderRefBuild> vertexRefMap; // Vertex shader reference build infomations
            std::unordered_map<std::string, ShaderRefBuild> pixelRefMap; // Pixel shader reference build infomations
            ShaderRefBuild* currentShaderRef;
            std::shared_ptr<EffectTechnique> currentTech;
            PassBuild* currentPass;
            RenderSystem& renderSystem;
            ResourceManager& resourceManager;

            BuildContext(RenderSystem& rs, ResourceManager& rm)
                : renderSystem(rs)
                , resourceManager(rm)
            {
                // Add reserved parameters
                params.emplace("_WorldMatrix");
                params.emplace("_ViewMatrix");
                params.emplace("_ProjMatrix");
            }

            template <class T>
            std::unordered_map<std::string, ShaderRefBuild>& shaderRefMap();

            template <>
            std::unordered_map<std::string, ShaderRefBuild>& shaderRefMap<VertexShader>()
            {
                return vertexRefMap;
            }

            template <>
            std::unordered_map<std::string, ShaderRefBuild>& shaderRefMap<PixelShader>()
            {
                return pixelRefMap;
            }
        };

        // Parse tree
        struct ParseNode : TreeNode<std::shared_ptr<ParseNode>>
        {
            std::unordered_map<std::string, std::function<void(ParseContext&)>> parserMap;

            template <class T>
            void mapChildParser(const std::string& key)
            {
                parserMap.emplace(key, [&](ParseContext& context)
                {
                    std::shared_ptr<T> child = std::make_shared<T>();
                    child->parse(context);
                    addChild(std::move(child));
                });
            }

            void executeChildParser(const std::string& key, ParseContext& context)
            {
                const auto parseFun = parserMap.find(key);
                assert(parseFun != std::cend(parserMap) && "Material parse error.");
                parseFun->second(context);
            }

            virtual void parse(ParseContext& context) {}
            virtual void build(BuildContext& context) {}
        };

        // Parse "float4" element
        struct Elem_float4 : ParseNode
        {
            std::string paramName;

            void parse(ParseContext& context)
            {
                // Store parameter name
                paramName = *context.currentToken;
                context.currentToken += 2;
            }

            void build(BuildContext& context)
            {
                // Add parameter into context
                const auto check = context.params.emplace(paramName);
                assert(check.second && "Conflict parameters.");
            }
        };

        // Parse "parameters" block
        struct Block_parameters : ParseNode
        {
            Block_parameters()
            {
                mapChildParser<Elem_float4>("float4");
            }

            void parse(ParseContext& context)
            {
                assert(*context.currentToken == "{" && "Material parse error.");
                ++context.currentToken;

                while (*context.currentToken != "}")
                {
                    const auto token = *context.currentToken;
                    ++context.currentToken;
                    executeChildParser(token, context);
                }

                ++context.currentToken;
            }

            void build(BuildContext& context)
            {
                for (const auto c : getChildren())
                {
                    c->build(context);
                }
            }
        };

        // Parse "compile" element
        struct Elem_compile : ParseNode
        {
            std::string path;

            void parse(ParseContext& context)
            {
                assert(*context.currentToken == "(" && "Material parse error.");
                ++context.currentToken;

                // Store shader path
                path = *context.currentToken;
                context.currentToken += 3;
            }

            void build(BuildContext& context)
            {
                // Set path into current builder
                context.currentShaderRef->path = path;
            }
        };

        // Parse "map" element
        struct Elem_map : ParseNode
        {
            std::string materialParam; // key
            std::string shaderParam;

            void parse(ParseContext& context)
            {
                assert(*context.currentToken == "(" && "Material parse error.");
                ++context.currentToken;

                // Store key and value (material parameter and shader constant) 
                materialParam = *context.currentToken;
                context.currentToken += 2;
                shaderParam = *context.currentToken;
                context.currentToken += 3;
            }

            void build(BuildContext& context)
            {
                assert(context.params.find(materialParam) != std::cend(context.params) && "Not exist material parameter.");

                // Add mapping into current builder
                const auto check = context.currentShaderRef->mappings.emplace(materialParam, shaderParam);
                assert(check.second && "Conflict material parameter.");
            }
        };

        // Parse "vertex_shader" and "pixel_shader" block
        template <class Shader>
        struct Block_shader : ParseNode
        {
            std::string refName;

            Block_shader()
            {
                mapChildParser<Elem_compile>("compile");
                mapChildParser<Elem_map>("map");
            }

            void parse(ParseContext& context)
            {
                // Store reference name
                refName = *context.currentToken;
                context.currentToken += 2;

                while (*context.currentToken != "}")
                {
                    const auto token = *context.currentToken;
                    ++context.currentToken;
                    executeChildParser(token, context);
                }
                ++context.currentToken;
            }

            void build(BuildContext& context)
            {
                // Set current builder
                ShaderRefBuild ref;
                context.currentShaderRef = &ref;

                for (const auto c : getChildren())
                {
                    c->build(context);
                }

                // Add builder into context
                const auto check = context.shaderRefMap<Shader>().emplace(refName, ref);
                assert(check.second && "Conflict shader reference.");
            }
        };

        // Parse "vertex_shader_ref" and "pixel_shader_ref" element
        template <class Shader>
        struct Elem_shaderRef : ParseNode
        {
            std::string useShader;

            void parse(ParseContext& context)
            {
                assert(*context.currentToken == "(" && "Material parse error.");
                ++context.currentToken;

                // Store reference shader
                useShader = *context.currentToken;
                context.currentToken += 3;
            }

            void build(BuildContext& context)
            {
                // Set shader reference into current builder
                context.currentPass->shaderRef<Shader>() =
                    context.shaderRefMap<Shader>()[useShader].make<Shader>(context.renderSystem, context.resourceManager);
            }
        };

        // Parse "pass" block
        struct Block_pass : ParseNode
        {
            size_t index;

            Block_pass()
            {
                mapChildParser<Elem_shaderRef<VertexShader>>("vertex_shader_ref");
                mapChildParser<Elem_shaderRef<PixelShader>>("pixel_shader_ref");
            }

            void parse(ParseContext& context)
            {
                // Store index
                index = std::stoi(*context.currentToken);
                context.currentToken += 2;

                while (*context.currentToken != "}")
                {
                    const auto token = *context.currentToken;
                    ++context.currentToken;
                    executeChildParser(token, context);
                }
                ++context.currentToken;
            }

            void build(BuildContext& context)
            {
                // Set current builder
                PassBuild pass;
                context.currentPass = &pass;

                for (const auto c : getChildren())
                {
                    c->build(context);
                }

                // Add pass into current technique
                context.currentTech->addPass(index, pass.make(context.renderSystem));
            }
        };

        // Parse "technique" block
        struct Block_technique : ParseNode
        {
            std::string name;

            Block_technique()
            {
                mapChildParser<Block_pass>("pass");
            }

            void parse(ParseContext& context)
            {
                // Store name
                name = *context.currentToken;
                context.currentToken += 2;

                while (*context.currentToken != "}")
                {
                    const auto token = *context.currentToken;
                    ++context.currentToken;
                    executeChildParser(token, context);
                }
                ++context.currentToken;
            }

            void build(BuildContext& context)
            {
                // Set current technique
                const auto tech = std::make_shared<EffectTechnique>();
                context.currentTech = tech;

                for (const auto c : getChildren())
                {
                    c->build(context);
                }

                // Add technique into material
                context.material->addTechnique(name, tech);
            }
        };

        // Root parser
        struct RootParser : ParseNode
        {
            RootParser()
            {
                mapChildParser<Block_parameters>("parameters");
                mapChildParser<Block_shader<VertexShader>>("vertex_shader");
                mapChildParser<Block_shader<PixelShader>>("pixel_shader");
                mapChildParser<Block_technique>("technique");
            }

            void parse(ParseContext& context)
            {
                while (context.currentToken != context.tokenEnd)
                {
                    const auto token = *context.currentToken;
                    ++context.currentToken;
                    executeChildParser(token, context);
                }
            }

            void build(BuildContext& context)
            {
                context.material = std::make_shared<Material>();
                for (const auto c : getChildren())
                {
                    c->build(context);
                }
            }
        };

        // Erase commands from source code
        void eraseComments(std::string& source)
        {
            // Erase "//"
            for (auto p = source.find("//"); p != std::string::npos; p = source.find("//", p))
            {
                auto end = source.find('\n', p + 2);
                if (end == std::string::npos)
                {
                    end = source.length();
                }
                source.erase(p, end - p + 1);
            }

            // Erase "/*  */"
            for (auto p = source.find("/*"); p != std::string::npos; p = source.find("/*", p))
            {
                auto end = source.find("*/", p + 2);
                assert(end != std::string::npos && "You need close comment.");

                source.erase(p, end - p + 2);
                source.insert(p, " ");
            }
        }

        // Delimiters definition
        const std::string delims = " \t\n,(){};";

        // Step position by find nonspace character
        size_t stepByNonspace(size_t p, const std::string& str)
        {
            while (p < str.length() && std::isspace(str[p]))
            {
                ++p;
            }
            return p;
        }

        // Tokenized source code by delimiters
        std::vector<std::string> tokenized(const std::string& source)
        {
            std::vector<std::string> tokens;

            for (auto p = stepByNonspace(0, source); p < source.length(); p = stepByNonspace(p, source))
            {
                auto found = source.find_first_of(delims, p);
                if (found == std::string::npos)
                {
                    found = source.length();
                }

                if (p == found)
                {
                    tokens.emplace_back(source.substr(p, 1));
                    p = found + 1;
                }
                else
                {
                    tokens.emplace_back(source.substr(p, found - p));
                    p = found;
                }
            }

            return tokens;
        }
    }

    std::shared_ptr<Material> loadMaterial(RenderSystem& renderSystem, ResourceManager& resourceManager, const std::string& path)
    {
        // Open file
        std::ifstream file(path);
        enforce<FileException>(file.is_open(), "Failed to open file (" + path + ").");

        // Erase commands from source code
        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        eraseComments(source);

        // Tokenized source code
        const auto tokens = tokenized(source);

        // Parse
        ParseContext parseContext;
        parseContext.currentToken = std::cbegin(tokens);
        parseContext.tokenEnd = std::cend(tokens);

        RootParser parser;
        parser.parse(parseContext);

        // Create material
        BuildContext buildContext(renderSystem, resourceManager);
        parser.build(buildContext);

        return buildContext.material;
    }
}