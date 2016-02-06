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
        std::shared_ptr<InputLayout> inputLayout_;

    public:
        /** Shader model */
        static const std::string model;

        /** Constructs with a byte code */
        explicit VertexShader(ID3DBlob* byteCode);

        /** Returns input layout of this shader */
        std::shared_ptr<InputLayout> getInputLayout() const;
    };
}

#endif