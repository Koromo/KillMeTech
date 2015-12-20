#ifndef _KILLME_SHADER_H_
#define _KILLME_SHADER_H_

#include "d3dsupport.h"
#include "../core/string.h"
#include "../windows/winsupport.h"
#include <d3dcompiler.h>
#include <memory>

namespace killme
{
    /** Shader type definition */
    enum class ShaderType
    {
        vertex,
        pixel
    };

    namespace detail
    {
        // Shader basic implementation
        template <ShaderType Type>
        class BasicShader
        {
        private:
            ComUniquePtr<ID3DBlob> byteCode_;

        public:
            // Construct with a shader byte code
            explicit BasicShader(ID3DBlob* byteCode)
                : byteCode_(makeComUnique(byteCode))
            {
            }

            // Returns shader binary
            const void* getByteCode() const
            {
                return byteCode_->GetBufferPointer();
            }

            size_t getByteCodeSize() const
            {
                return byteCode_->GetBufferSize();
            }
        };
    }

    /** Each shader definition */
    using VertexShader = detail::BasicShader<ShaderType::vertex>;
    using PixelShader = detail::BasicShader<ShaderType::pixel>;

    namespace detail
    {
        template <ShaderType Type>
        const char* shaderModel();

        template <>
        inline const char* shaderModel<ShaderType::vertex>()
        {
            return "vs_5_0";
        }

        template <>
        inline const char* shaderModel<ShaderType::pixel>()
        {
            return "ps_5_0";
        }
    }

    /** Compile shader from file */
    template <ShaderType Type>
    std::shared_ptr<detail::BasicShader<Type>> compileShader(const tstring& filename)
    {
        ID3DBlob* code;
        ID3DBlob* err;

        const auto hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "main", detail::shaderModel<Type>(), 0, 0, &code, &err);
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

        return std::make_shared<detail::BasicShader<Type>>(code);
    }
}

#endif