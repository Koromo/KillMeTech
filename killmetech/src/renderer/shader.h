#ifndef _KILLME_SHADER_H_
#define _KILLME_SHADER_H_

#include "d3dsupport.h"
#include "../core/utility.h"
#include "../core/string.h"
#include "../core/optional.h"
#include "../windows/winsupport.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

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
    };

    /** The basic implementation for each shaders */
    class BasicShader
    {
    private:
        ComUniquePtr<ID3DBlob> byteCode_;
        ComUniquePtr<ID3D12ShaderReflection> reflection_;
        D3D12_SHADER_DESC desc_;

    public:
        /** Constructs with a byte code */
        explicit BasicShader(ID3DBlob* byteCode);

        /** For drived classes */
        ~BasicShader() = default;

        /** Returns the byte code */
        const void* getByteCode() const;
        size_t getByteCodeSize() const;

        /** Returns the Direct3D input signature */
        std::vector<D3D12_SIGNATURE_PARAMETER_DESC> getD3DInputSignature();

        /** Returns constant buffers */
        std::vector<ConstantBufferDescription> describeConstnatBuffers();
    };

    /** Compile a shader from file */
    template <class Shader>
    std::shared_ptr<Shader> compileShader(const tstring& filename)
    {
        ID3DBlob* code;
        ID3DBlob* err;

        const auto hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "main", Shader::model.c_str(), 0, 0, &code, &err);
        if (FAILED(hr))
        {
            std::string msg = "Failed to compile shader (" + narrow(filename) + ").";
            if (err)
            {
                msg += "\n";
                msg += static_cast<char*>(err->GetBufferPointer());
                err->Release();
            }
            throw Direct3DException(msg);
        }

        return std::make_shared<Shader>(code);
    }
}

#endif