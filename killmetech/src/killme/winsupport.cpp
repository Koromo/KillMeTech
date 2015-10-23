#include "winsupport.h"

namespace killme
{
    WindowsException::WindowsException(const std::string& msg)
        : Exception(msg)
    {
    }
}