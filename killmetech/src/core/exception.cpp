#include "exception.h"

namespace killme
{
    std::function<void()> detail::ScopeExit::relay;

    detail::ScopeExit::ScopeExit(std::function<void()> fun)
        : fun_(fun)
    {
    }

    detail::ScopeExit::~ScopeExit()
    {
        fun_();
    }

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

    ItemNotFoundException::ItemNotFoundException(const std::string& msg)
        : Exception(msg)
    {
    }

    InvalidArgmentException::InvalidArgmentException(const std::string& msg)
        : Exception(msg)
    {
    }
}