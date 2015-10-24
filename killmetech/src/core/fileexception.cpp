#include "fileexception.h"

namespace killme
{
    FileException::FileException(const std::string& msg)
        : Exception(msg)
    {
    }
}