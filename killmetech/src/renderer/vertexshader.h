#ifndef _KILLME_VERTEXSHADER_H_
#define _KILLME_VERTEXSHADER_H_

#include "shader.h"
#include <d3d12.h>
#include <d3dcompiler.h>
#include <vector>

namespace killme
{
    /** Vertex shader */
    class VertexShader : public BasicShader
    {
    private:
        ComUniquePtr<ID3D12ShaderReflection> reflection_;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElems_;
        D3D12_INPUT_LAYOUT_DESC inputLayout_;

    public:
        /** Shader model definition */
        static const std::string model;

        /** Construct with a byte code */
        explicit VertexShader(ID3DBlob* byteCode);

        /** Returns Direct3D input layout */
        /// NOTE: Input slot start from 0
        D3D12_INPUT_LAYOUT_DESC getD3DInputLayout() const;
    };
}

#endif