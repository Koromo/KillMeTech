#include "exception.h"

namespace killme
{
    std::function<void()> detail::ScopeExit::relay;

    Exception::Exception(const std::string& msg)
        : msg_(msg)
    {
    }

    std::string Exception::getMessage() const
    {
        return msg_;
    }

    const char* Exception::what() const
    {
        return msg_.c_str();
    }

    FileException::FileException(const std::string& msg)
        : Exception(msg)
    {
    }
}