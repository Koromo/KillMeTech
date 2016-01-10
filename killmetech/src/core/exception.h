#ifndef _KILLME_EXCEPTION_H_
#define _KILLME_EXCEPTION_H_

#include <exception>
#include <string>
#include <utility>
#include <functional>

/** Generate unique id */
#ifdef __COUNTER__
#define KILLME_ID __COUNTER__
#elif
#define KILLME_ID __LINE__
#endif

/** Cat a and b */
#define KILLME_CAT(a, b) a##b

/** Scope guard */
#define KILLME_SCOPE_EXIT_NAME(id) KILLME_CAT(killme_scope_exit, id)
#define KILLME_SCOPE_EXIT \
    const killme::detail::ScopeExit KILLME_SCOPE_EXIT_NAME(KILLME_ID) = killme::detail::ScopeExit::relay = [&]

namespace killme
{
    namespace detail
    {
        // For KILLME_SCOPE_EXIT
        struct ScopeExit
        {
            static std::function<void()> relay;
            std::function<void()> fun_;
            ScopeExit(std::function<void()> fun) : fun_(fun) {}
            ~ScopeExit() { fun_(); }
        };
    }

    /** Returns "value" if it is not 0 object, Otherwise throw exception "E" */
    template <class E, class T, class... Args>
    T enforce(T value, Args&&... args)
    {
        if (!value)
        {
            throw E(std::forward<Args>(args)...);
        }
        return value;
    }

    /** Root exception class of KillMeTech */
    class Exception : public std::exception
    {
    private:
        std::string msg_;

    public:
        /** Construct with a message */
        explicit Exception(const std::string& msg) : msg_(msg) {}

        /** Returns message */
        std::string getMessage() const { return msg_; }

        /** Returns message that is same to the Exception::getMessage() */
        const char* what() const { return msg_.c_str(); }
    };

	/** File relational exception */
	class FileException : public Exception
	{
	public:
		/** Construct with a message */
        explicit FileException(const std::string& msg) : Exception(msg) {}
	};
}

#endif