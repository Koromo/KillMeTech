#include "exception.h"

namespace killme
{
    std::function<void()> detail::ScopeExit::relay;
}