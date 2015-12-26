#include "shader.h"

namespace killme
{
    BasicShader::BasicShader(ID3DBlob* byteCode)
        : byteCode_(byteCode)
    {
    }

    const void*  BasicShader::getByteCode() const
    {
        return byteCode_->GetBufferPointer();
    }

    size_t  BasicShader::getByteCodeSize() const
    {
        return byteCode_->GetBufferSize();
    }
}