#ifndef _KILLME_EXCEPTION_H_
#define _KILLME_EXCEPTION_H_

#include <exception>
#include <string>
#include <utility>
#include <functional>

/** For KILLME_SCOPE_GUARD */
#ifdef __COUNTER__
#define KILLME_ID __COUNTER__
#elif
#define KILLME_ID __LINE__
#endif

/** For KILLME_SCOPE_GUARD */
#define KILLME_CAT(a, b) a##b

/** The scope guard statement */
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

    /** The basic exception of the KillMe Tech API */
    class Exception : public std::exception
    {
    private:
        std::string msg_;

    public:
        /** Constructs */
        explicit Exception(const std::string& msg);

        Exception(const Exception&) = default;
        Exception(Exception&&) = default;

        /** Assignment operator */
        Exception& operator =(const Exception&) = default;
        Exception& operator =(Exception&&) = default;

        /** Returns the message */
        std::string getMessage() const;

        /** Returns the message that is same to the Exception::getMessage() */
        const char* what() const;
    };

	/** The file relational exception */
	class FileException : public Exception
	{
	public:
		/** Constructs with a message */
        explicit FileException(const std::string& msg);
	};
}

#endif