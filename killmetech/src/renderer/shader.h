#ifndef _KILLME_SHADER_H_
#define _KILLME_SHADER_H_

#include "d3dsupport.h"
#include "../core/utility.h"
#include "../core/string.h"
#include "../core/optional.h"
#include "../core/exception.h"
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
        pixel
    };

    /** Resource type definitions */
    enum class BoundResourceType
    {
        cbuffer,
        texture,
        sampler
    };

    namespace detail
    {
        D3D_SHADER_INPUT_TYPE toD3DSIType(BoundResourceType type);
    }

    /** The bound resource description */
    class BoundResourceDescription
    {
    private:
        D3D12_SHADER_INPUT_BIND_DESC desc_;

    public:
        /** Construct */
        explicit BoundResourceDescription(const D3D12_SHADER_INPUT_BIND_DESC& desc);

        /** Return resource type */
        BoundResourceType getType() const;

        /** Return name */
        std::string getName() const;

        /** Return register slot */
        size_t getRegisterSlot() const;
    };

    /** The variable description */
    struct VariableDescription
    {
        size_t size;
        size_t offset;
        std::shared_ptr<const unsigned char> defaultValue;
    };

    /** The constant buffer description */
    class ConstantBufferDescription : public BoundResourceDescription
    {
    private:
        size_t size_;
        std::unordered_map<std::string, VariableDescription> variables_;

    public:
        /** Constructs with a reflection of constant buffer */
        ConstantBufferDescription(ID3D12ShaderReflectionConstantBuffer* reflection, const D3D12_SHADER_INPUT_BIND_DESC& boundDesc);

        /** Returns the size of buffer */
        size_t getSize() const;

        /** Returns the description of variable */
        Optional<VariableDescription> describeVariable(const std::string& name) const;

        /** Returns descriptions of variable */
        auto describeVariables() const
            -> decltype(makeRange(variables_))
        {
            return makeRange(variables_);
        }
    };

    /** The basic implementation for each shaders */
    class BasicShader : public IsResource
    {
    private:
        ComUniquePtr<ID3DBlob> byteCode_;
        ComUniquePtr<ID3D12ShaderReflection> reflection_;
        D3D12_SHADER_DESC desc_;

    public:
        /** Constructs with a byte code */
        explicit BasicShader(ID3DBlob* byteCode);

        /** Returns the byte code */
        const void* getByteCode() const;
        size_t getByteCodeSize() const;

        /** Returns the Direct3D input signature */
        auto getD3DInputSignature()
            -> decltype(makeRange(std::vector<D3D12_SIGNATURE_PARAMETER_DESC>()))
        {
            std::vector<D3D12_SIGNATURE_PARAMETER_DESC> params(desc_.InputParameters);
            for (size_t i = 0; i < desc_.InputParameters; ++i)
            {
                enforce<Direct3DException>(
                    SUCCEEDED(reflection_->GetInputParameterDesc(i, &(params[i]))),
                    "Failed to get the description of input parameter of shader.");
            }
            return makeRange(std::move(params));
        }

        /** Return bound resource description */
        Optional<BoundResourceDescription> describeBoundResource(const std::string& name);

        /** Return bound resource descriptions */
        auto describeBoundResources(BoundResourceType type)
            -> decltype(makeRange(std::vector<BoundResourceDescription>()))
        {
            const auto& d3dDescs = describeD3DBoundResources(detail::toD3DSIType(type));

            std::vector<BoundResourceDescription> descs;
            descs.reserve(d3dDescs.length());
            for (const auto& d3dDesc : d3dDescs)
            {
                descs.emplace_back(d3dDesc);
            }

            return makeRange(std::move(descs));
        }

        /** Return constant buffer descriptions */
        auto describeConstnatBuffers()
            -> decltype(makeRange(std::vector<ConstantBufferDescription>()))
        {
            const auto& d3dResourceDescs = describeD3DBoundResources(D3D_SIT_CBUFFER);

            std::vector<ConstantBufferDescription> cbuffers;
            cbuffers.reserve(d3dResourceDescs.length());
            for (const auto& d3dResourceDesc : d3dResourceDescs)
            {
                // Get the reflection of the constant buffer
                const auto cbuffer = enforce<Direct3DException>(
                    reflection_->GetConstantBufferByName(d3dResourceDesc.Name),
                    "Failed to get the reflection of the constant buffer.");
                cbuffers.emplace_back(cbuffer, d3dResourceDesc);
            }

            return makeRange(std::move(cbuffers));
        }

    private:
        auto describeD3DBoundResources(D3D_SHADER_INPUT_TYPE type)
            -> decltype(makeRange(std::vector<D3D12_SHADER_INPUT_BIND_DESC>()))
        {
            std::vector<D3D12_SHADER_INPUT_BIND_DESC> descs;
            descs.reserve(desc_.BoundResources);

            for (UINT i = 0; i < desc_.BoundResources; ++i)
            {
                // Get i'th resource description
                D3D12_SHADER_INPUT_BIND_DESC desc;
                enforce<Direct3DException>(
                    SUCCEEDED(reflection_->GetResourceBindingDesc(i, &desc)),
                    "Failed to get the bound resource description.");

                if (desc.Type == type)
                {
                    descs.emplace_back(desc);
                }
            }

            return makeRange(std::move(descs));
        }
    };

    /** Compile a shader from file */
    template <class Shader>
    std::shared_ptr<Shader> compileShader(const tstring& path)
    {
        ID3DBlob* code;
        ID3DBlob* err = NULL;

        const auto hr = D3DCompileFromFile(path.c_str(), nullptr, nullptr, "main", Shader::model.c_str(), 0, 0, &code, &err);
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