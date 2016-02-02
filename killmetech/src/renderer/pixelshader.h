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
        /** Shader model definition */
        static const std::string model;

        /** Construct with a byte code */
        explicit PixelShader(ID3DBlob* byteCode);
    };
}

#endif