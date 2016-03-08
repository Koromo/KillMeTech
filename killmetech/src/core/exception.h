#ifndef _KILLME_EXCEPTION_H_
#define _KILLME_EXCEPTION_H_

#include "utility.h"
#include <exception>
#include <string>
#include <utility>
#include <functional>

/** Scope guard statement */
#define KILLME_SCOPE_EXIT_NAME(id) KILLME_CAT(killme_scope_exit, id)
#define KILLME_SCOPE_EXIT \
    const killme::detail::ScopeExit KILLME_SCOPE_EXIT_NAME(KILLME_ID) = killme::detail::ScopeExit::relay = [&]

namespace killme
{
    namespace detail
    {
        // For KILLME_SCOPE_GUARD
        struct ScopeExit
        {
            static std::function<void()> relay;
            std::function<void()> fun_;
            ScopeExit(std::function<void()> fun);
            ~ScopeExit();
        };
    }

    /** Return the "value" if it is not 0 object, Otherwise throw the exception "E" */
    template <class E, class T, class... Args>
    T enforce(T value, Args&&... args)
    {
        if (!value)
        {
            throw E(std::forward<Args>(args)...);
        }
        return value;
    }

    /** Basic exception of the KillMe Tech API */
    class Exception : public std::exception
    {
    private:
        std::string msg_;

    public:
        /** Construct */
        explicit Exception(const std::string& msg);
        Exception(const Exception&) = default;
        Exception(Exception&&) = default;

        /** Assignment operator */
        Exception& operator =(const Exception&) = default;
        Exception& operator =(Exception&&) = default;

        /** Return the message */
        std::string getMessage() const;

        /** Return the message that is same to the Exception::getMessage() */
        const char* what() const;
    };

	/** File relational exception */
	class FileException : public Exception
	{
	public:
		/** Construct */
        explicit FileException(const std::string& msg);
	};

    /** Item not found exception */
    class ItemNotFoundException : public Exception
    {
    public:
        /** Construct */
        explicit ItemNotFoundException(const std::string& msg);
    };

    /** Invalid argment exception */
    class InvalidArgmentException : public Exception
    {
    public:
        /** Construct */
        explicit InvalidArgmentException(const std::string& msg);
    };
}

#endif