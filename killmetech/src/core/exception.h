#ifndef _KILLME_EXCEPTION_H_
#define _KILLME_EXCEPTION_H_

#include <exception>
#include <string>
#include <utility>
#include <functional>

/** Generate an unique id */
#ifdef __COUNTER__
#define KILLME_ID __COUNTER__
#elif
#define KILLME_ID __LINE__
#endif

/** Cat a and b */
#define KILLME_CAT(a, b) a##b

/** The scope guard */
#define KILLME_SCOPE_EXIT_NAME(id) KILLME_CAT(killme_scope_exit, id)
#define KILLME_SCOPE_EXIT \
    const killme::detail::ScopeExit KILLME_SCOPE_EXIT_NAME(KILLME_ID) = killme::detail::ScopeExit::relay = [&]

namespace killme
{
    namespace detail
    {
        // For the KILLME_SCOPE_EXIT
        struct ScopeExit
        {
            static std::function<void()> relay;
            std::function<void()> fun_;
            ScopeExit(std::function<void()> fun) : fun_(fun) {}
            ~ScopeExit() { fun_(); }
        };
    }

    /** Returns the "value" if it is not 0 object, Otherwise throw the exception "E" */
    template <class E, class T, class... Args>
    T enforce(T value, Args&&... args)
    {
        if (!value)
        {
            throw E(std::forward<Args>(args)...);
        }
        return value;
    }

    /** The basic exception of the KillMeTech */
    class Exception : public std::exception
    {
    private:
        std::string msg_;

    public:
        /** Constructs with a message */
        explicit Exception(const std::string& msg);

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