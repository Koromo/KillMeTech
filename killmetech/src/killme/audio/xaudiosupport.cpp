#include "xaudiosupport.h"

namespace killme
{
    XAudioException::XAudioException(const std::string& msg)
        : Exception(msg)
    {
    }
}