#ifndef _KILLME_VERTEXSHADER_H_
#define _KILLME_VERTEXSHADER_H_

#include "shader.h"
#include <d3dcompiler.h>
#include <memory>

namespace killme
{
    class InputLayout;

    /** Vertex shader */
    class VertexShader : public BasicShader
    {
    private:
        std::shared_ptr<InputLayout> inputLayout_;

    public:
        /** Shader model */
        static const std::string model;

        /** Construct with a byte code */
        explicit VertexShader(ID3DBlob* byteCode);

        /** Return the input layout of the shader */
        std::shared_ptr<InputLayout> getInputLayout() const;
    };
}

#endif