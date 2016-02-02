#ifndef _KILLME_VERTEXSHADER_H_
#define _KILLME_VERTEXSHADER_H_

#include "shader.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <memory>

namespace killme
{
    class InputLayout;

    /** Vertex shader */
    class VertexShader : public BasicShader
    {
    private:
        ComUniquePtr<ID3D12ShaderReflection> reflection_;
        std::shared_ptr<InputLayout> inputLayout_;

    public:
        /** Shader model definition */
        static const std::string model;

        /** Construct with a byte code */
        explicit VertexShader(ID3DBlob* byteCode);

        /** Returns input layout */
        /// NOTE: Input slot start from 0
        std::shared_ptr<InputLayout> getInputLayout() const;
    };
}

#endif