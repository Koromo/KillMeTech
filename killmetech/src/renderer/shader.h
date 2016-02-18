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

    /** The variable description */
    struct VariableDescription
    {
        size_t size;
        size_t offset;
        std::shared_ptr<const unsigned char> defaultValue;
    };

    /** The constant buffer description */
    class ConstantBufferDescription
    {
    private:
        std::string name_;
        size_t registerSlot_;
        size_t size_;
        std::unordered_map<std::string, VariableDescription> variables_;

    public:
        /** Constructs with a reflection of constant buffer */
        ConstantBufferDescription(ID3D12ShaderReflectionConstantBuffer* reflection, size_t registerSlot);

        /** Retruns the buffer name */
        std::string getName() const;

        /** Returns the size of buffer */
        size_t getSize() const;

        /** Returns the register slot */
        size_t getRegisterSlot() const;

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

        /** Returns constant buffers */
        auto describeConstnatBuffers()
            -> decltype(makeRange(std::vector<ConstantBufferDescription>()))
        {
            std::vector<ConstantBufferDescription> cbuffers;
            cbuffers.reserve(desc_.ConstantBuffers);

            for (UINT i = 0; i < desc_.BoundResources; ++i)
            {
                // Get the description of the i'th resource
                D3D12_SHADER_INPUT_BIND_DESC resourceDesc;
                enforce<Direct3DException>(
                    SUCCEEDED(reflection_->GetResourceBindingDesc(i, &resourceDesc)),
                    "Failed to get the description of the resource.");

                if (resourceDesc.Type == D3D_SIT_CBUFFER)
                {
                    // Get the reflection of the constant buffer
                    const auto cbuffer = enforce<Direct3DException>(
                        reflection_->GetConstantBufferByName(resourceDesc.Name),
                        "Failed to get the reflection of the constant buffer.");
                    cbuffers.emplace_back(cbuffer, resourceDesc.BindPoint);
                }
            }

            return makeRange(std::move(cbuffers));
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