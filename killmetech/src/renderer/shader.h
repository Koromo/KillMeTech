#ifndef _KILLME_SHADER_H_
#define _KILLME_SHADER_H_

#include "d3dsupport.h"
#include "../core/string.h"
#include "../windows/winsupport.h"
#include <d3dcompiler.h>
#include <memory>

namespace killme
{
    /** Shader type definitions */
    enum class ShaderType
    {
        vertex,
        pixel
    };

    /** Basic implementation of each shaders */
    class BasicShader
    {
    protected:
        ComUniquePtr<ID3DBlob> byteCode_;

    public:
        /** Constructs with a byte code */
        explicit BasicShader(ID3DBlob* byteCode) : byteCode_(makeComUnique(byteCode)) {}

        /** For drived classes */
        ~BasicShader() = default;

        /** Returns a byte code */
        const void* getByteCode() const { return byteCode_->GetBufferPointer(); }
        size_t getByteCodeSize() const { return byteCode_->GetBufferSize(); }
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