#ifndef _KILLME_SHADER_H_
#define _KILLME_SHADER_H_

#include "d3dsupport.h"
#include "../core/utility.h"
#include "../core/string.h"
#include "../core/optional.h"
#include "../core/exception.h"
#include "../core/platform.h"
#include "../resources/resource.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

namespace killme
{
    /** Shader type definitions */
    enum class ShaderType
    {
        vertex,
        pixel,
        geometry,
        compute
    };

    /** Bound resource type definitions */
    enum class BoundResourceType
    {
        cbuffer,
        texture,
        sampler,
        bufferRW
    };

    /** Bound resource description */
    class BoundResourceDescription
    {
    private:
        D3D12_SHADER_INPUT_BIND_DESC desc_;

    public:
        /** Construct */
        BoundResourceDescription() = default;
        explicit BoundResourceDescription(const D3D12_SHADER_INPUT_BIND_DESC& desc);

        /** Return the resource type */
        BoundResourceType getType() const;

        /** Return the name */
        std::string getName() const;

        /** Return register slot */
        size_t getRegisterSlot() const;
    };

    /** Variable description */
    struct VariableDescription
    {
        size_t size;
        size_t offset;
        std::shared_ptr<const unsigned char> init;
    };

    /** Constant buffer description */
    class ConstantBufferDescription : public BoundResourceDescription
    {
    private:
        size_t size_;
        std::unordered_map<std::string, VariableDescription> variables_;

    public:
        /** Construct */
        ConstantBufferDescription() = default;
        ConstantBufferDescription(ID3D12ShaderReflectionConstantBuffer* reflection, const D3D12_SHADER_INPUT_BIND_DESC& boundDesc);

        /** Return the size of buffer */
        size_t getSize() const;

        /** Return the description of variable */
        Optional<VariableDescription> describeVariable(const std::string& name) const;

        /** Return the descriptions of variable */
        auto describeVariables() const
            -> decltype(constRange(variables_))
        {
            return constRange(variables_);
        }
    };

    /** Basic implementation for each shaders */
    class BasicShader : public IsResource
    {
    private:
        ShaderType type_;
        ComUniquePtr<ID3DBlob> byteCode_;
        ComUniquePtr<ID3D12ShaderReflection> reflection_;
        D3D12_SHADER_DESC desc_;

    public:
        /** For drived classes */
        virtual ~BasicShader() = default;

        /** Return shader type */
        ShaderType getType() const;

        /** Return the byte code */
        D3D12_SHADER_BYTECODE getD3DByteCode() const;

        /** Return the count of bound resources */
        size_t getNumBoundResources() const;

        /** Return the Direct3D input signature */
        auto getD3DInputSignature()
            -> decltype(emplaceRange(std::vector<D3D12_SIGNATURE_PARAMETER_DESC>()))
        {
            std::vector<D3D12_SIGNATURE_PARAMETER_DESC> params(desc_.InputParameters);
            for (size_t i = 0; i < desc_.InputParameters; ++i)
            {
                reflection_->GetInputParameterDesc(i, &(params[i]));
            }
            return emplaceRange(std::move(params));
        }

        /** Return bound resource description */
        Optional<BoundResourceDescription> describeBoundResource(const std::string& name);

        /** Return constant buffer description */
        Optional<ConstantBufferDescription> describeConstantBuffer(const std::string& name);

        /** Return bound resource descriptions */
        auto describeBoundResources(BoundResourceType type)
            -> decltype(emplaceRange(std::vector<BoundResourceDescription>()))
        {
            const auto& d3dDescs = describeD3DBoundResources(D3DMappings::toD3DShaderInputType(type));

            std::vector<BoundResourceDescription> descs;
            descs.reserve(std::cend(d3dDescs) - std::cbegin(d3dDescs));
            for (const auto& d3dDesc : d3dDescs)
            {
                descs.emplace_back(d3dDesc);
            }

            return emplaceRange(std::move(descs));
        }

        /** Return constant buffer descriptions */
        auto describeConstnatBuffers()
            -> decltype(emplaceRange(std::vector<ConstantBufferDescription>()))
        {
            const auto& d3dResourceDescs = describeD3DBoundResources(D3D_SIT_CBUFFER);

            std::vector<ConstantBufferDescription> cbuffers;
            cbuffers.reserve(std::cend(d3dResourceDescs) - std::cbegin(d3dResourceDescs));

            for (const auto& d3dResourceDesc : d3dResourceDescs)
            {
                const auto cbuffer = reflection_->GetConstantBufferByName(d3dResourceDesc.Name);
                cbuffers.emplace_back(cbuffer, d3dResourceDesc);
            }

            return emplaceRange(std::move(cbuffers));
        }


    protected:
        /** Construct with a byte code */
        BasicShader(ShaderType type, ID3DBlob* byteCode);

    private:
        auto describeD3DBoundResources(D3D_SHADER_INPUT_TYPE type)
            -> decltype(emplaceRange(std::vector<D3D12_SHADER_INPUT_BIND_DESC>()))
        {
            std::vector<D3D12_SHADER_INPUT_BIND_DESC> descs;
            descs.reserve(desc_.BoundResources);

            for (UINT i = 0; i < desc_.BoundResources; ++i)
            {
                D3D12_SHADER_INPUT_BIND_DESC desc;
                reflection_->GetResourceBindingDesc(i, &desc);

                if (desc.Type == type)
                {
                    descs.emplace_back(desc);
                }
            }

            return emplaceRange(std::move(descs));
        }
    };

    /** Vertex shader */
    class VertexShader : public BasicShader
    {
    private:
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElems_;
        D3D12_INPUT_LAYOUT_DESC inputLayout_;

    public:
        /** Shader model */
        static const std::string MODEL;

        /** Entry point */
        static const std::string ENTRY;

        /** Construct with a byte code */
        explicit VertexShader(ID3DBlob* byteCode);

        /** Return the input layout of the shader */
        D3D12_INPUT_LAYOUT_DESC getD3DInputLayout() const;
    };

    /** Pixel shader */
    class PixelShader : public BasicShader
    {
    public:
        /** Shader model */
        static const std::string MODEL;

        /** Entry point */
        static const std::string ENTRY;

        /** Construct with a byte code */
        explicit PixelShader(ID3DBlob* byteCode);
    };

    /** Geometry shader */
    class GeometryShader : public BasicShader
    {
    public:
        /** Shader model */
        static const std::string MODEL;

        /** Entry point */
        static const std::string ENTRY;

        /** Construct with a byte code */
        explicit GeometryShader(ID3DBlob* byteCode);
    };

    /** Compute shader */
    class ComputeShader : public BasicShader
    {
    public:
        /** Shader model */
        static const std::string MODEL;

        /** Entry point */
        static const std::string ENTRY;

        /** Construct with a byte code */
        explicit ComputeShader(ID3DBlob* byteCode);
    };

    /** Compile a shader from file */
    template <class Shader>
    std::shared_ptr<Shader> compileHlslShader(const tstring& path)
    {
        ID3DBlob* code;
        ID3DBlob* err = NULL;

#ifdef KILLME_DEBUG
        const auto flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#elif
        const auto flags = 0u;
#endif

        const auto hr = D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, Shader::ENTRY.c_str(), Shader::MODEL.c_str(), flags, 0, &code, &err);
        if (FAILED(hr))
        {
            if (err)
            {
                const std::string msg = static_cast<char*>(err->GetBufferPointer());
                err->Release();
                throw Direct3DException(msg);
            }
            else
            {
                throw FileException("File not found (" + narrow(path) + ").");
            }
        }

        return std::make_shared<Shader>(code);
    }
}

#endif