#include "pixelshader.h"

namespace killme
{
    const std::string PixelShader::model = "ps_5_0";

    PixelShader::PixelShader(ID3DBlob* byteCode)
        : BasicShader(byteCode)
    {
    }
}