#include "d3dsupport.h"

namespace killme
{
    Direct3DException::Direct3DException(const std::string& msg)
        : Exception(msg)
    {
    }
}