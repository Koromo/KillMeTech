#ifndef _KILLME_PIXELSHADER_H_
#define _KILLME_PIXELSHADER_H_

#include "shader.h"
#include <d3dcompiler.h>

namespace killme
{
    /** Pixel shader */
    class PixelShader : public BasicShader
    {
    public:
        /** Shader model */
        static const std::string model;

        /** Constructs with a byte code */
        explicit PixelShader(ID3DBlob* byteCode);
    };
}

#endif