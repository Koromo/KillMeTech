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

    /** Bound resource type definitions */
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

    /** Bound resource description */
    class BoundResourceDescription
    {
    private:
        D3D12_SHADER_INPUT_BIND_DESC desc_;

    public:
        /** Construct */
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
        /** Construct with a reflection of constant buffer */
        ConstantBufferDescription(ID3D12ShaderReflectionConstantBuffer* reflection, const D3D12_SHADER_INPUT_BIND_DESC& boundDesc);

        /** Return the size of buffer */
        size_t getSize() const;

        /** Return the description of variable */
        Optional<VariableDescription> describeVariable(const std::string& name) const;

        /** Return the descriptions of variable */
        auto describeVariables() const
            -> decltype(makeRange(variables_))
        {
            return makeRange(variables_);
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
        /** Construct with a byte code */
        BasicShader(ShaderType type, ID3DBlob* byteCode);

        /** Return shader type */
        ShaderType getType() const;

        /** Return the byte code */
        const void* getByteCode() const;
        size_t getByteCodeSize() const;

        /** Return the Direct3D input signature */
        auto getD3DInputSignature()
            -> decltype(makeRange(std::vector<D3D12_SIGNATURE_PARAMETER_DESC>()))
        {
            std::vector<D3D12_SIGNATURE_PARAMETER_DESC> params(desc_.InputParameters);
            for (size_t i = 0; i < desc_.InputParameters; ++i)
            {
                reflection_->GetInputParameterDesc(i, &(params[i]));
            }
            return makeRange(std::move(params));
        }

        /** Return bound resource description */
        Optional<BoundResourceDescription> describeBoundResource(const std::string& name);

        /** Return constant buffer description */
        Optional<ConstantBufferDescription> describeConstantBuffer(const std::string& name);

        /** Return bound resource descriptions */
        auto describeBoundResources(BoundResourceType type)
            -> decltype(makeRange(std::vector<BoundResourceDescription>()))
        {
            const auto& d3dDescs = describeD3DBoundResources(detail::toD3DSIType(type));

            std::vector<BoundResourceDescription> descs;
            descs.reserve(std::cend(d3dDescs) - std::cbegin(d3dDescs));
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
            cbuffers.reserve(std::cend(d3dResourceDescs) - std::cbegin(d3dResourceDescs));

            for (const auto& d3dResourceDesc : d3dResourceDescs)
            {
                const auto cbuffer = reflection_->GetConstantBufferByName(d3dResourceDesc.Name);
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
                D3D12_SHADER_INPUT_BIND_DESC desc;
                reflection_->GetResourceBindingDesc(i, &desc);

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
    std::shared_ptr<Shader> compileHlslShader(const tstring& path)
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