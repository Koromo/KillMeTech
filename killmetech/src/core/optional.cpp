#include "optional.h"

namespace killme
{
    OptionalException::OptionalException(const std::string& msg)
        : Exception(msg)
    {
    }

    const detail::NullOpt nullopt;
}